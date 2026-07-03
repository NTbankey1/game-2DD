#include "CollisionSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include <algorithm>

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

} // namespace engine::physics
