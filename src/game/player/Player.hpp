#pragma once
#include "core/core.hpp"

namespace game {
struct PlayerComponent {
    core::Vec2f velocity{};
    bool isGrounded = false;
};
} // namespace game
