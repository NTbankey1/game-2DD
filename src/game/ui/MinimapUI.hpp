#pragma once

#include "core/core.hpp"
#include <vector>
#include <cstdint>

namespace engine::renderer { class TextRenderer; }

namespace game {

/// Tracks discovered areas and renders a minimap overlay.
class MinimapUI {
public:
    explicit MinimapUI(engine::renderer::TextRenderer& text);

    /// Mark current camera center as explored.
    void Discover(core::Vec2f worldPos, float viewW, float viewH, float worldW, float worldH);

    /// Draw minimap at screen position.
    void Render(float sx, float sy, float mapW, float mapH, core::Vec2f playerWorldPos, float worldW, float worldH);

private:
    engine::renderer::TextRenderer& m_text;
    static constexpr int GRID = 16; // 16x16 pixel minimap chunks
    int m_gridW = 0;
    int m_gridH = 0;
    std::vector<uint8_t> m_discovered; // 0=undiscovered, 1=discovered
};

} // namespace game
