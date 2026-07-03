#include "CollisionSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"

namespace engine::physics {

void CollisionSystem::FixedUpdate(entt::registry& registry, core::events::EventBus& eventBus) {
    auto view = registry.view<AABBComponent, engine::renderer::TransformComponent>();

    // Find GroundTag + AABB for ground plane
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

    for (auto entity : view) {
        if (entity == ground) continue;

        auto& aabb = view.get<AABBComponent>(entity);
        auto& transform = view.get<engine::renderer::TransformComponent>(entity);
        core::Rectf worldRect(transform.position + aabb.localBounds.position, aabb.localBounds.size);

        // Check ground collision
        if (ground != entt::null && worldRect.Overlaps(groundRect)) {
            // Push player out: snap to ground top
            float overlap = (worldRect.Bottom() - groundRect.Top());
            transform.position.y -= overlap;

            // Stop downward velocity
            if (registry.all_of<VelocityComponent>(entity)) {
                auto& vel = registry.get<VelocityComponent>(entity);
                if (vel.velocity.y > 0) vel.velocity.y = 0;
            }

            eventBus.Publish(CollisionEvent(entity, ground, core::Vec2f(0, overlap)));
        }
    }
}

} // namespace engine::physics
