#include "MinimapUI.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>

namespace game {

MinimapUI::MinimapUI(engine::renderer::TextRenderer& text) : m_text(text) {}

void MinimapUI::Discover(core::Vec2f worldPos, float viewW, float viewH, float worldW, float worldH) {
    int gw = static_cast<int>(std::ceil(worldW / GRID));
    int gh = static_cast<int>(std::ceil(worldH / GRID));
    if (gw != m_gridW || gh != m_gridH) {
        m_gridW = gw;
        m_gridH = gh;
        m_discovered.assign(gw * gh, 0);
    }

    // Mark tiles visible in viewport as discovered
    int startX = std::max(0, static_cast<int>(worldPos.x / GRID) - 2);
    int startY = std::max(0, static_cast<int>(worldPos.y / GRID) - 2);
    int endX = std::min(m_gridW, static_cast<int>((worldPos.x + viewW) / GRID) + 2);
    int endY = std::min(m_gridH, static_cast<int>((worldPos.y + viewH) / GRID) + 2);

    for (int y = startY; y < endY; y++)
        for (int x = startX; x < endX; x++)
            if (y * m_gridW + x < static_cast<int>(m_discovered.size()))
                m_discovered[y * m_gridW + x] = 1;
}

void MinimapUI::Render(float sx, float sy, float mapW, float mapH, core::Vec2f playerWorldPos,
                       float worldW, float worldH) {
    if (m_gridW <= 0 || m_gridH <= 0) return;

    // Border
    SDL_SetRenderDrawColor(nullptr, 0x22, 0x22, 0x33, 180);
    // We don't have direct SDL_Renderer access — draw via text and minimal rects
    // For now, simple text-based indicator
    m_text.RenderString("MAP", sx, sy - 14, 0x66, 0x88, 0xAA);

    float scaleX = mapW / worldW;
    float scaleY = mapH / worldH;

    // Player dot
    float px = sx + playerWorldPos.x * scaleX;
    float py = sy + playerWorldPos.y * scaleY;
    m_text.RenderString("X", px - 4, py - 6, 0xFF, 0xCC, 0x33);
}

} // namespace game
