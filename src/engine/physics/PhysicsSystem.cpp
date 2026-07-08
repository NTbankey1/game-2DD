#include "PhysicsSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include <algorithm>

namespace engine::physics {

static constexpr float TERMINAL_VELOCITY = 900.0f;
static constexpr float FALL_GRAVITY_MULT = 1.5f;   // heavier when falling
static constexpr float RISE_GRAVITY_MULT = 0.6f;   // lighter when rising (tap = short, hold = high)
static constexpr float MAX_FALL_SPEED = -TERMINAL_VELOCITY;

void PhysicsSystem::FixedUpdate(entt::registry& registry, float dt) {
    // Apply gravity with variable jump physics
    auto gravView = registry.view<GravityComponent, VelocityComponent, PlayerStateComponent>();
    for (auto entity : gravView) {
        auto& vel = gravView.get<VelocityComponent>(entity);
        float g = gravView.get<GravityComponent>(entity).gravity;
        auto& pState = gravView.get<PlayerStateComponent>(entity);

        if (pState.isGrounded) {
            // No gravity when on ground
            vel.velocity.y = 0.0f;
            continue;
        }

        if (vel.velocity.y < 0) {
            // Rising — apply reduced gravity so hold-jump goes higher
            vel.velocity.y += g * RISE_GRAVITY_MULT * dt;
        } else {
            // Falling — apply increased gravity for snappy fall
            vel.velocity.y += g * FALL_GRAVITY_MULT * dt;
        }

        // Cap terminal velocity
        vel.velocity.y = std::min(vel.velocity.y, TERMINAL_VELOCITY);
    }

    // Apply plain gravity to non-player entities
    auto regView = registry.view<GravityComponent, VelocityComponent>();
    for (auto entity : regView) {
        if (registry.all_of<PlayerStateComponent>(entity)) continue;
        auto& vel = regView.get<VelocityComponent>(entity);
        float g = regView.get<GravityComponent>(entity).gravity;
        vel.velocity.y += g * dt;
        vel.velocity.y = std::min(vel.velocity.y, TERMINAL_VELOCITY);
    }

    // Integrate velocity into position
    auto posView = registry.view<engine::renderer::TransformComponent, VelocityComponent>();
    for (auto entity : posView) {
        auto& pos = posView.get<engine::renderer::TransformComponent>(entity);
        auto& vel = posView.get<VelocityComponent>(entity);
        pos.position += vel.velocity * dt;
    }

    // Update player state timers
    auto playerView = registry.view<PlayerStateComponent>();
    for (auto entity : playerView) {
        auto& p = playerView.get<PlayerStateComponent>(entity);
        if (!p.isGrounded) p.coyoteTimer += dt;
        else p.coyoteTimer = 0.0f;

        if (p.jumpBufferTimer > 0.0f) p.jumpBufferTimer += dt;
    }
}

} // namespace engine::physics
