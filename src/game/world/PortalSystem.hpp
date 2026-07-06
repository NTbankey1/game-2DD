#pragma once

#include <entt/entt.hpp>
#include "core/core.hpp"

namespace game {

/// Simple portal system — checks proximity and teleports player.
/// Call after physics update each frame.
class PortalSystem {
public:
    /// Returns true and fills destination if player is near an active portal.
    bool CheckPortal(entt::registry& registry, core::Vec2f& outDest, float playerX, float playerY);
};

} // namespace game
