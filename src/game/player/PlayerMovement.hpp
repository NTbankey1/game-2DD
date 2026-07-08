#pragma once

#include <entt/entt.hpp>

namespace engine::physics { struct PlayerStateComponent; }

namespace game {

/// Player movement state enum
enum class MoveState {
    Idle,
    Running,
    Jumping,
    Falling,
    WallSliding,
    Dashing
};

/// Input snapshot for a single frame
struct PlayerInput {
    float hInput = 0.0f;
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool dashPressed = false;
};

/// Player movement system—handles walking, dashing, and wall-slide.
class PlayerMovement {
public:
    PlayerMovement();

    /// Call once per fixed-step. Updates velocity and state.
    void Update(entt::registry& registry, const PlayerInput& input, float dt);

    /// Access current movement state
    [[nodiscard]] MoveState GetState() const { return m_state; }
    [[nodiscard]] bool IsDashing() const { return m_dashTimer > 0.0f; }
    [[nodiscard]] float GetDashProgress() const { return m_dashTimer / DASH_DURATION; }

    /// Movement parameters (tweakable)
    static constexpr float WALK_SPEED = 280.0f;
    static constexpr float JUMP_VEL = -500.0f;
    static constexpr float COYOTE_MS = 0.07f;
    static constexpr float BUFFER_MS = 0.10f;
    static constexpr float DASH_SPEED = 600.0f;
    static constexpr float DASH_DURATION = 0.15f;
    static constexpr float DASH_COOLDOWN = 0.5f;
    static constexpr float WALL_SLIDE_FALL = 120.0f; // fall speed while sliding

private:
    MoveState m_state = MoveState::Idle;
    float m_dashTimer = 0.0f;
    float m_dashCooldown = 0.0f;
    float m_facingDir = 1.0f; // 1 for right, -1 for left
    bool m_canDash = true;

    void HandleWalk(entt::registry& registry, float hInput);
    void HandleDash(entt::registry& registry, float hInput, bool dashPressed, float dt);
    void HandleWallSlide(entt::registry& registry, float hInput, float dt);
    void UpdateState(entt::registry& registry, float hInput);
};

} // namespace game
