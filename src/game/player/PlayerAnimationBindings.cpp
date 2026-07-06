#include "PlayerAnimationBindings.hpp"
#include "PlayerMovement.hpp"
#include "engine/animation/AnimationController.hpp"

namespace game {

/// Maps game::MoveState → animation controller state names + updates sprite source rect.
/// This is the ONLY place that knows how "player" movement maps to animation frames.
void UpdatePlayerAnimation(
    engine::animation::AnimationController& animCtrl,
    MoveState moveState,
    bool isGrounded,
    float dt,
    float& frameOut,    // sprite sheet column for source rect
    float hInput)
{
    // Select animation state based on movement state
    switch (moveState) {
    case MoveState::Idle:
        animCtrl.SetState("Idle");
        break;
    case MoveState::Running:
        animCtrl.SetState("Run");
        break;
    case MoveState::Jumping:
        animCtrl.SetState("Jump");
        break;
    case MoveState::Falling:
        animCtrl.SetState("Fall");
        break;
    case MoveState::WallSliding:
        animCtrl.SetState("WallSlide");
        break;
    case MoveState::Dashing:
        animCtrl.SetState("Dash");
        break;
    }

    // Advance animation
    frameOut = static_cast<float>(animCtrl.Update(dt));
}

} // namespace game
