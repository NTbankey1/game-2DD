#include "PortalSystem.hpp"
#include "PortalComponent.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include <cmath>

namespace game {

bool PortalSystem::CheckPortal(entt::registry& registry, core::Vec2f& outDest, float playerX, float playerY) {
    for (auto [e, portal, pos] : registry.view<PortalComponent, engine::renderer::TransformComponent>().each()) {
        if (!portal.active) continue;
        float dx = playerX - pos.position.x;
        float dy = playerY - pos.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 25.0f) {
            outDest = portal.destination;
            return true;
        }
    }
    return false;
}

} // namespace game
