#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace engine::animation {

/// A single frame in an animation sequence
struct AnimFrame {
    int row = 0;       // sprite sheet row (0 = tile Y, for multi-row sheets)
    int col = 0;       // sprite sheet column (which frame in the sequence)
    float duration = 0.1f; // seconds to hold this frame
};

/// One named animation state (e.g. "Idle", "Run", "Jump")
struct AnimState {
    std::vector<AnimFrame> frames;
    bool loop = true;
};

/// Generic animation state machine.
/// Does NOT know about "player", "NPC", or any game concept.
/// Just maps state names to frame sequences and tracks transitions.
class AnimationController {
public:
    AnimationController() = default;

    /// Add a named animation state. Overwrites if name exists.
    void AddState(const std::string& name, const AnimState& state);

    /// Transition to a named state. If same as current, no-op.
    /// If different, starts crossfade from previous frame.
    void SetState(const std::string& name);

    /// Advance animation timer. Returns current frame index (col).
    /// If transitioning, returns blended frame (lerp toward next).
    int Update(float dt);

    /// Get current state name
    [[nodiscard]] const std::string& GetCurrentState() const { return m_currentState; }

    /// Returns true if a transition is in progress
    [[nodiscard]] bool IsTransitioning() const { return m_crossfadeTimer > 0.0f; }

    /// Crossfade duration in seconds (default 0.05s)
    void SetCrossfadeDuration(float d) { m_crossfadeDuration = d; }

    /// Get progress (0-1) of current state's animation cycle
    [[nodiscard]] float GetCycleProgress() const;

    /// Set sprite sheet columns (for computing source rect)
    void SetFrameCols(int cols) { m_frameCols = cols; }
    [[nodiscard]] int GetFrameCols() const { return m_frameCols; }

    /// Get the sprite sheet column for the current frame
    [[nodiscard]] int GetCurrentCol() const { return m_currentCol; }
    [[nodiscard]] int GetPreviousCol() const { return m_previousCol; }
    [[nodiscard]] float GetBlendFactor() const { return m_blendFactor; }

private:
    std::unordered_map<std::string, AnimState> m_states;
    std::string m_currentState;
    std::string m_previousState;

    int m_currentCol = 0;
    int m_previousCol = 0;
    float m_timer = 0.0f;
    int m_frameIndex = 0;

    float m_crossfadeTimer = 0.0f;
    float m_crossfadeDuration = 0.05f;
    float m_blendFactor = 0.0f;

    int m_frameCols = 4; // default: 4-column sprite sheet
};

} // namespace engine::animation
