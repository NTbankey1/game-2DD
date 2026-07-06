#include "CollisionSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/tilemap/Tilemap.hpp"
#include <algorithm>
#include <cmath>

namespace engine::physics {

void CollisionSystem::FixedUpdate(entt::registry& registry, core::events::EventBus& eventBus, bool& playerDead, int& score) {
    auto view = registry.view<AABBComponent, engine::renderer::TransformComponent>();

    // Find ground
    entt::entity ground = entt::null;
    core::Rectf groundRect;
    for (auto e : registry.view<GroundTag>()) {
        if (registry.all_of<AABBComponent, engine::renderer::TransformComponent>(e)) {
            ground = e;
            auto& gAabb = registry.get<AABBComponent>(e);
            auto& gPos = registry.get<engine::renderer::TransformComponent>(e);
            groundRect = core::Rectf(gPos.position + gAabb.localBounds.position, gAabb.localBounds.size);
        }
    }

    // Find player
    entt::entity player = entt::null;
    core::Rectf playerRect;
    for (auto e : registry.view<PlayerTag>()) {
        if (registry.all_of<AABBComponent, engine::renderer::TransformComponent>(e)) {
            player = e;
            auto& pAabb = registry.get<AABBComponent>(e);
            auto& pPos = registry.get<engine::renderer::TransformComponent>(e);
            playerRect = core::Rectf(pPos.position + pAabb.localBounds.position, pAabb.localBounds.size);
        }
    }

    // Reset grounded state — will be set to true if collision found
    if (player != entt::null && registry.all_of<PlayerStateComponent>(player)) {
        registry.get<PlayerStateComponent>(player).isGrounded = false;
    }

    // Check player-ground collision
    if (player != entt::null && ground != entt::null) {
        if (playerRect.Overlaps(groundRect)) {
            auto& pos = registry.get<engine::renderer::TransformComponent>(player);
            float overlap = playerRect.Bottom() - groundRect.Top();
            pos.position.y -= overlap;

            if (registry.all_of<VelocityComponent>(player)) {
                auto& vel = registry.get<VelocityComponent>(player);
                if (vel.velocity.y > 0) vel.velocity.y = 0;
            }
            if (registry.all_of<PlayerStateComponent>(player)) {
                registry.get<PlayerStateComponent>(player).isGrounded = true;
            }
            eventBus.Publish(CollisionEvent(player, ground, core::Vec2f(0, overlap)));
        }
    }

    // Update player rect after ground collision resolution
    if (player != entt::null) {
        auto& pAabb = registry.get<AABBComponent>(player);
        auto& pPos = registry.get<engine::renderer::TransformComponent>(player);
        playerRect = core::Rectf(pPos.position + pAabb.localBounds.position, pAabb.localBounds.size);

        // Reset wall contact
        if (registry.all_of<PlayerStateComponent>(player)) {
            auto& ps = registry.get<PlayerStateComponent>(player);
            ps.wallContactLeft = false;
            ps.wallContactRight = false;
        }

        // Tilemap collision
        if (m_tilemap) {
            ResolveTileCollision(registry, player, pPos.position,
                                 pAabb.localBounds.size.x * 0.5f,
                                 pAabb.localBounds.size.y * 0.5f);
            playerRect = core::Rectf(pPos.position + pAabb.localBounds.position, pAabb.localBounds.size);
        }
    }

    // Check obstacle collisions (ground vs obstacles, player vs obstacles)
    for (auto entity : view) {
        if (entity == ground || entity == player) continue;

        auto& aabb = view.get<AABBComponent>(entity);
        auto& transform = view.get<engine::renderer::TransformComponent>(entity);
        core::Rectf worldRect(transform.position + aabb.localBounds.position, aabb.localBounds.size);

        // Obstacle vs ground
        if (ground != entt::null && worldRect.Overlaps(groundRect)) {
            float overlap = worldRect.Bottom() - groundRect.Top();
            transform.position.y -= overlap;
            if (registry.all_of<VelocityComponent>(entity)) {
                auto& vel = registry.get<VelocityComponent>(entity);
                if (vel.velocity.y > 0) vel.velocity.y = 0;
            }
        }

        // Player vs obstacle
        if (player != entt::null && playerRect.Overlaps(worldRect)) {
            playerDead = true;
            eventBus.Publish(PlayerDiedEvent(score));
        }
    }
}

void CollisionSystem::ResolveTileCollision(entt::registry& registry, entt::entity player,
                                            core::Vec2f& pos, float hw, float hh) const {
    if (!m_tilemap) return;
    int ts = m_tilemap->GetTileSize();
    float tsF = static_cast<float>(ts);

    // Player AABB in world space
    float left = pos.x - hw;
    float right = pos.x + hw;
    float top = pos.y;
    float bottom = pos.y + hh * 2.0f;

    // Tile range the player overlaps
    int tileLeft = static_cast<int>(std::floor(left / tsF));
    int tileRight = static_cast<int>(std::floor((right - 1.0f) / tsF));
    int tileTop = static_cast<int>(std::floor(top / tsF));
    int tileBottom = static_cast<int>(std::floor((bottom - 1.0f) / tsF));

    // Check each overlapping tile
    for (int ty = tileTop; ty <= tileBottom; ty++) {
        for (int tx = tileLeft; tx <= tileRight; tx++) {
            if (!m_tilemap->IsSolid(tx, ty)) continue;

            // Tile AABB in world space
            float tileLeftW = static_cast<float>(tx) * tsF;
            float tileRightW = tileLeftW + tsF;
            float tileTopW = static_cast<float>(ty) * tsF;
            float tileBottomW = tileTopW + tsF;

            // Calculate overlap on each axis
            float overlapLeft = right - tileLeftW;
            float overlapRight = tileRightW - left;
            float overlapTop = bottom - tileTopW;
            float overlapBottom = tileBottomW - top;

            // Resolve smallest overlap first
            float minOverlapX = std::min(overlapLeft, overlapRight);
            float minOverlapY = std::min(overlapTop, overlapBottom);

            if (minOverlapX < minOverlapY) {
                // Push horizontally — detect wall contact
                if (overlapLeft < overlapRight) {
                    pos.x -= overlapLeft;
                    if (auto* ps = registry.try_get<PlayerStateComponent>(player))
                        ps->wallContactRight = true;
                } else {
                    pos.x += overlapRight;
                    if (auto* ps = registry.try_get<PlayerStateComponent>(player))
                        ps->wallContactLeft = true;
                }
                if (auto* vel = registry.try_get<VelocityComponent>(player)) {
                    vel->velocity.x = 0;
                }
            } else {
                // Push vertically
                if (overlapTop < overlapBottom) {
                    pos.y -= overlapTop;
                    if (auto* vel = registry.try_get<VelocityComponent>(player)) {
                        if (vel->velocity.y > 0) vel->velocity.y = 0;
                    }
                    if (auto* ps = registry.try_get<PlayerStateComponent>(player)) {
                        ps->isGrounded = true;
                    }
                } else {
                    pos.y += overlapBottom;
                    if (auto* vel = registry.try_get<VelocityComponent>(player)) {
                        if (vel->velocity.y < 0) vel->velocity.y = 0;
                    }
                }
            }

            // Recompute player rect edges after push
            left = pos.x - hw;
            right = pos.x + hw;
            top = pos.y;
            bottom = pos.y + hh * 2.0f;
        }
    }
}

} // namespace engine::physics
