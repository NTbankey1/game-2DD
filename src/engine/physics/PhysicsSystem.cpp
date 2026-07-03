#include "PhysicsSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"

namespace engine::physics {

void PhysicsSystem::FixedUpdate(entt::registry& registry, float dt) {
    // Apply gravity to all entities with both components
    auto gravView = registry.view<GravityComponent, VelocityComponent>();
    for (auto entity : gravView) {
        auto& vel = gravView.get<VelocityComponent>(entity);
        float g = gravView.get<GravityComponent>(entity).gravity;
        vel.velocity.y += g * dt;
    }

    // Integrate velocity into position
    auto posView = registry.view<engine::renderer::TransformComponent, VelocityComponent>();
    for (auto entity : posView) {
        auto& pos = posView.get<engine::renderer::TransformComponent>(entity);
        auto& vel = posView.get<VelocityComponent>(entity);
        pos.position += vel.velocity * dt;
    }
}

} // namespace engine::physics
