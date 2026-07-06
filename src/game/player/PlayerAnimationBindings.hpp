#pragma once

namespace engine::animation { class AnimationController; }
namespace game { enum class MoveState; }

/// Single translation layer: maps game::MoveState to animation state names.
/// This is the only place in the codebase that knows how "player" movement
/// maps to animation frames.
namespace game {
void UpdatePlayerAnimation(
    engine::animation::AnimationController& animCtrl,
    MoveState moveState,
    bool isGrounded,
    float dt,
    float& frameOut,
    float hInput);
}
