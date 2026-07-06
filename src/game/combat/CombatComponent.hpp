#pragma once

#include "core/core.hpp"

namespace game {

/// Entity that can deal damage on overlap
struct HitboxComponent {
    int damage = 1;
    float knockbackForce = 200.0f;
};

/// Entity that can receive damage
struct HurtboxComponent {
    bool invulnerable = false;
    float invulnTimer = 0.0f;
};

} // namespace game
