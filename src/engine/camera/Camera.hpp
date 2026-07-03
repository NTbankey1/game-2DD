#pragma once

#include "core/core.hpp"

namespace engine {

class Camera {
public:
    Camera() = default;

    void SetPosition(core::Vec2f pos) noexcept { m_position = pos; }
    [[nodiscard]] core::Vec2f GetPosition() const noexcept { return m_position; }

    void Move(core::Vec2f delta) noexcept { m_position += delta; }

    void SetZoom(float zoom) noexcept { m_zoom = zoom; }
    [[nodiscard]] float GetZoom() const noexcept { return m_zoom; }

    void SetViewport(core::Vec2f size) noexcept { m_viewport = size; }
    [[nodiscard]] core::Vec2f GetViewport() const noexcept { return m_viewport; }

    [[nodiscard]] core::Rectf GetBounds() const noexcept {
        return core::Rectf(m_position, m_viewport * m_zoom);
    }

    [[nodiscard]] core::Vec2f GetParallaxOffset(float factor) const noexcept {
        return m_position * factor;
    }

private:
    core::Vec2f m_position{};
    core::Vec2f m_viewport{1280.0f, 720.0f};
    float m_zoom = 1.0f;
};

} // namespace engine
