#pragma once

#include <entt/entt.hpp>
#include "core/core.hpp"

namespace game {

/// A portal that teleports the player to a destination.
struct PortalComponent {
    core::Vec2f destination{};
    bool active = true;
};

} // namespace game
