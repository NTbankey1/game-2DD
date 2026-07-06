#include "AnimationController.hpp"
#include <algorithm>

namespace engine::animation {

void AnimationController::AddState(const std::string& name, const AnimState& state) {
    m_states[name] = state;
    if (m_currentState.empty()) {
        m_currentState = name;
    }
}

void AnimationController::SetState(const std::string& name) {
    if (m_currentState == name) return;
    if (m_states.find(name) == m_states.end()) return;

    m_previousState = m_currentState;
    m_previousCol = m_currentCol;
    m_currentState = name;
    m_frameIndex = 0;
    m_timer = 0.0f;
    m_crossfadeTimer = m_crossfadeDuration;
    m_blendFactor = 0.0f;
}

int AnimationController::Update(float dt) {
    // Update crossfade
    if (m_crossfadeTimer > 0.0f) {
        m_crossfadeTimer -= dt;
        if (m_crossfadeTimer <= 0.0f) {
            m_crossfadeTimer = 0.0f;
            m_blendFactor = 0.0f;
        } else {
            m_blendFactor = m_crossfadeTimer / m_crossfadeDuration;
        }
    }

    auto it = m_states.find(m_currentState);
    if (it == m_states.end()) return m_currentCol;

    const auto& state = it->second;
    if (state.frames.empty()) return m_currentCol;

    // Advance timer
    m_timer += dt;
    const auto& currentFrame = state.frames[m_frameIndex];
    if (m_timer >= currentFrame.duration) {
        m_timer -= currentFrame.duration;
        m_frameIndex++;
        if (m_frameIndex >= static_cast<int>(state.frames.size())) {
            if (state.loop) {
                m_frameIndex = 0;
            } else {
                m_frameIndex = static_cast<int>(state.frames.size()) - 1;
            }
        }
    }

    m_currentCol = state.frames[m_frameIndex].col;
    return m_currentCol;
}

float AnimationController::GetCycleProgress() const {
    auto it = m_states.find(m_currentState);
    if (it == m_states.end() || it->second.frames.empty()) return 0.0f;
    float totalDuration = 0.0f;
    for (const auto& f : it->second.frames) totalDuration += f.duration;
    if (totalDuration <= 0.0f) return 0.0f;
    float t = m_timer;
    for (int i = 0; i <= m_frameIndex && i < static_cast<int>(it->second.frames.size()); i++) {
        t += it->second.frames[i].duration;
    }
    return std::min(t / totalDuration, 1.0f);
}

} // namespace engine::animation
