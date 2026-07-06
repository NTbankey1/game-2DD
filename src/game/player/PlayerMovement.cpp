#include "PlayerMovement.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/tilemap/Tilemap.hpp"
#include <algorithm>
#include <cmath>

namespace game {

PlayerMovement::PlayerMovement() = default;

void PlayerMovement::Update(entt::registry& registry, const PlayerInput& input, float dt) {
    if (input.hInput != 0.0f) m_facingDir = input.hInput > 0.0f ? 1.0f : -1.0f;

    HandleWalk(registry, input.hInput);
    HandleDash(registry, input.hInput, input.dashPressed, dt);
    HandleWallSlide(registry, input.hInput, dt);
    UpdateState(registry, input.hInput);

    // Dash cooldown
    if (m_dashCooldown > 0.0f) m_dashCooldown -= dt;
    if (m_dashTimer > 0.0f) m_dashTimer -= dt;

    // Determine state
    bool grounded = false;
    float vy = 0.0f;
    for (auto e : registry.view<engine::physics::PlayerTag>()) {
        if (auto* ps = registry.try_get<engine::physics::PlayerStateComponent>(e)) {
            grounded = ps->isGrounded;
        }
        if (auto* vel = registry.try_get<engine::physics::VelocityComponent>(e)) {
            vy = vel->velocity.y;
        }
    }

    if (m_dashTimer > 0.0f) {
        m_state = MoveState::Dashing;
    } else if (m_state == MoveState::WallSliding && vy < 0.0f) {
        m_state = MoveState::Falling;
    } else if (!grounded && vy < 0.0f) {
        m_state = MoveState::Falling;
    } else if (!grounded) {
        m_state = MoveState::Jumping;
    } else if (std::abs(input.hInput) > 0.1f) {
        m_state = MoveState::Running;
    } else {
        m_state = MoveState::Idle;
    }
}

void PlayerMovement::HandleWalk(entt::registry& registry, float hInput) {
    for (auto e : registry.view<engine::physics::PlayerTag>()) {
        auto& vel = registry.get<engine::physics::VelocityComponent>(e);
        if (m_dashTimer <= 0.0f) {
            vel.velocity.x = hInput * WALK_SPEED;
        }
    }
}

void PlayerMovement::HandleDash(entt::registry& registry, float hInput, bool dashPressed, float dt) {
    if (dashPressed && m_dashCooldown <= 0.0f && m_dashTimer <= 0.0f) {
        m_dashTimer = DASH_DURATION;
        m_dashCooldown = DASH_COOLDOWN;
        for (auto e : registry.view<engine::physics::PlayerTag>()) {
            auto& vel = registry.get<engine::physics::VelocityComponent>(e);
            vel.velocity.x = m_facingDir * DASH_SPEED;
            vel.velocity.y = 0.0f;
        }
    }

    if (m_dashTimer > 0.0f) {
        for (auto e : registry.view<engine::physics::PlayerTag>()) {
            auto& vel = registry.get<engine::physics::VelocityComponent>(e);
            vel.velocity.x = m_facingDir * DASH_SPEED;
            vel.velocity.y = 0.0f;
        }
    }
}

void PlayerMovement::HandleWallSlide(entt::registry& registry, float hInput, float dt) {
    // Wall-slide detection: player is against a wall and falling
    for (auto e : registry.view<engine::physics::PlayerTag>()) {
        auto& vel = registry.get<engine::physics::VelocityComponent>(e);
        auto* ps = registry.try_get<engine::physics::PlayerStateComponent>(e);

        if (ps && !ps->isGrounded && vel.velocity.y > 0.0f) {
            // Check if player is pressing toward a wall
            // This is checked by collision system via the wall-slide state
            // For now, simple version: near wall + holding into it
            bool nearWall = false;
            if (auto* aabb = registry.try_get<engine::physics::AABBComponent>(e)) {
                auto* pos = registry.try_get<engine::renderer::TransformComponent>(e);
                if (pos) {
                    float px = pos->position.x;
                    int ts = 32;
                    float sideX = px + (hInput * 4.0f) + aabb->localBounds.size.x * 0.5f;
                    int tx = static_cast<int>(sideX) / ts;
                    int ty1 = static_cast<int>(pos->position.y) / ts;
                    int ty2 = static_cast<int>(pos->position.y + aabb->localBounds.size.y) / ts;

                    // Use m_tilemap via collision system? For now simple heuristic
                    // If hInput pushes into solid area and player is falling
                    if (std::abs(hInput) > 0.5f) {
                        nearWall = true; // Will be refined with tilemap access
                    }
                }
            }

            if (nearWall && std::abs(hInput) > 0.5f) {
                // Wall-slide: slow fall
                vel.velocity.y = std::min(vel.velocity.y, WALL_SLIDE_FALL);
                m_state = MoveState::WallSliding;
            }
        }
    }
}

void PlayerMovement::UpdateState(entt::registry& /*registry*/, float hInput) {
    // State is determined in Update() after all handlers
    // This is a placeholder for future state transition logic
    (void)hInput;
}

} // namespace game
