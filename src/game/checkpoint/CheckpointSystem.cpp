#include "CheckpointSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include <cmath>

namespace game {

void CheckpointSystem::CheckActivation(entt::registry& registry, float playerX, float playerY) {
    for (auto [e, cp, pos] : registry.view<CheckpointComponent, engine::renderer::TransformComponent>().each()) {
        if (cp.activated) continue;
        float dx = playerX - pos.position.x;
        float dy = playerY - pos.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 30.0f) {
            cp.activated = true;
            cp.respawnPos = pos.position;
        }
    }
}

core::Vec2f CheckpointSystem::GetRespawnPos(const entt::registry& registry) const {
    for (auto [e, cp] : registry.view<CheckpointComponent>().each()) {
        if (cp.activated) return cp.respawnPos;
    }
    return m_defaultRespawn;
}

} // namespace game
