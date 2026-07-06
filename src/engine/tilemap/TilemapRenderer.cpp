#include "Tilemap.hpp"
#include <SDL3/SDL.h>

namespace engine::tilemap {

static void DrawFilledRect(SDL_Renderer* r, float x, float y, float s, uint8_t r_, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(r, r_, g, b, 0xFF);
    SDL_FRect rect{x, y, s, s};
    SDL_RenderFillRect(r, &rect);
}

static void DrawTileBorder(SDL_Renderer* r, float x, float y, float s, uint8_t r_, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(r, r_, g, b, 0x40);
    SDL_FRect rect{x, y, s, s};
    SDL_RenderRect(r, &rect);
}

void RenderTilemap(SDL_Renderer* renderer, const Tilemap& tilemap, const core::Vec2f& cameraPos,
                   const core::Vec2f& viewport) {
    int ts = tilemap.GetTileSize();
    float tsF = static_cast<float>(ts);

    // Visible tile range
    int startX = std::max(0, static_cast<int>(cameraPos.x) / ts - 1);
    int startY = std::max(0, static_cast<int>(cameraPos.y) / ts - 1);
    int endX = std::min(tilemap.GetWidth(), static_cast<int>(cameraPos.x + viewport.x) / ts + 2);
    int endY = std::min(tilemap.GetHeight(), static_cast<int>(cameraPos.y + viewport.y) / ts + 2);

    for (int layer = 0; layer < tilemap.LayerCount(); layer++) {
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                int tile = tilemap.GetTile(layer, x, y);
                if (tile == 0) continue;

                float sx = static_cast<float>(x) * tsF - cameraPos.x;
                float sy = static_cast<float>(y) * tsF - cameraPos.y;

                switch (static_cast<TileID>(tile)) {
                case TileID::GrassTop:
                    DrawFilledRect(renderer, sx, sy, tsF, 0x44, 0xBB, 0x44); // green
                    DrawTileBorder(renderer, sx, sy, tsF, 0x33, 0x99, 0x33);
                    break;
                case TileID::Ground:
                    DrawFilledRect(renderer, sx, sy, tsF, 0x8B, 0x6B, 0x4A); // dirt brown
                    DrawTileBorder(renderer, sx, sy, tsF, 0x6B, 0x4A, 0x2A);
                    break;
                case TileID::Platform:
                    DrawFilledRect(renderer, sx, sy, tsF, 0x66, 0x66, 0x88); // stone gray
                    DrawTileBorder(renderer, sx, sy, tsF, 0x44, 0x44, 0x66);
                    break;
                case TileID::Rock:
                    DrawFilledRect(renderer, sx, sy, tsF, 0x55, 0x55, 0x55); // dark rock
                    DrawTileBorder(renderer, sx, sy, tsF, 0x33, 0x33, 0x33);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

} // namespace engine::tilemap
