#include "InteractionSystem.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include <algorithm>
#include <cmath>

namespace game {

bool InteractionSystem::CheckProximity(entt::registry& registry, entt::entity& outTarget) {
    entt::entity player = entt::null;
    for (auto e : registry.view<engine::physics::PlayerTag>()) {
        player = e;
        break;
    }
    if (player == entt::null || !registry.all_of<engine::renderer::TransformComponent>(player))
        return false;

    auto& pPos = registry.get<engine::renderer::TransformComponent>(player);
    float nearestDist = 50.0f; // interaction range
    outTarget = entt::null;

    for (auto e : registry.view<InteractableTag>()) {
        if (!registry.all_of<engine::renderer::TransformComponent>(e)) continue;
        auto& ePos = registry.get<engine::renderer::TransformComponent>(e);
        float dx = ePos.position.x - pPos.position.x;
        float dy = ePos.position.y - pPos.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < nearestDist) {
            nearestDist = dist;
            outTarget = e;
        }
    }
    return outTarget != entt::null;
}

float InteractionSystem::GetNearestDist(entt::registry& registry) {
    entt::entity dummy;
    if (CheckProximity(registry, dummy)) return 0.0f;
    return -1.0f;
}

} // namespace game
