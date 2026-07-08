#pragma once

#include "core/core.hpp"
#include <entt/entt.hpp>

namespace engine {

class Camera {
public:
    Camera() = default;

    void SetPosition(core::Vec2f pos) { m_position = pos; }
    core::Vec2f GetPosition() const noexcept { return m_position; }

    void Move(core::Vec2f delta) { m_position += delta; }

    void SetZoom(float zoom) noexcept { m_zoom = zoom; }
    float GetZoom() const noexcept { return m_zoom; }

    void SetViewport(core::Vec2f size) noexcept { m_viewport = size; }
    core::Vec2f GetViewport() const noexcept { return m_viewport; }

    core::Rectf GetBounds() const {
        return core::Rectf(m_position, m_viewport * m_zoom);
    }

    // Follow a target entity with lerp smoothing
    void FollowTarget(entt::entity target, const core::Vec2f& targetPos, float lerpFactor) {
        m_followTarget = target;
        m_lerpFactor = lerpFactor;
        // Smoothly move camera toward target
        core::Vec2f desired = targetPos - m_viewport * 0.5f;
        m_position = m_position + (desired - m_position) * m_lerpFactor;
        // Clamp to world bounds if set
        if (m_hasBounds) {
            m_position.x = std::max(m_bounds.position.x,
                std::min(m_position.x, m_bounds.position.x + m_bounds.size.x - m_viewport.x));
            m_position.y = std::max(m_bounds.position.y,
                std::min(m_position.y, m_bounds.position.y + m_bounds.size.y - m_viewport.y));
        }
    }

    void SetBounds(core::Rectf worldBounds) {
        m_bounds = worldBounds;
        m_hasBounds = true;
    }

    void ClearBounds() {
        m_hasBounds = false;
    }

    entt::entity GetFollowTarget() const noexcept { return m_followTarget; }

private:
    core::Vec2f m_position{};
    core::Vec2f m_viewport{1280.0f, 720.0f};
    float m_zoom = 1.0f;
    entt::entity m_followTarget = entt::null;
    float m_lerpFactor = 0.05f;
    core::Rectf m_bounds{};
    bool m_hasBounds = false;
};

} // namespace engine
