#include "TextRenderer.hpp"
#include "BitmapFont.hpp"
#include <cstring>
#include <cstdint>

namespace engine::renderer {

TextRenderer::TextRenderer(SDL_Renderer* renderer) : m_renderer(renderer) {
    for (int i = 0; i < MAX_CACHE; i++) {
        m_cache[i].texture = nullptr;
    }
}

TextRenderer::~TextRenderer() {
    ClearCache();
}

void TextRenderer::ClearCache() {
    for (int i = 0; i < m_cacheCount; i++) {
        if (m_cache[i].texture) {
            SDL_DestroyTexture(m_cache[i].texture);
            m_cache[i].texture = nullptr;
        }
    }
    m_cacheCount = 0;
}

SDL_Texture* TextRenderer::CreateTexture(const std::string& text, uint8_t r, uint8_t g, uint8_t b) {
    if (text.empty()) return nullptr;

    int w = static_cast<int>(text.size()) * FONT_CHAR_WIDTH;
    int h = FONT_CHAR_HEIGHT;

    // Create surface for the text
    SDL_Surface* surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) return nullptr;

    // Lock surface for pixel access
    SDL_LockSurface(surface);
    auto* pixels = static_cast<uint32_t*>(surface->pixels);
    int pitch = surface->pitch / 4;  // pixels per row

    uint32_t fgColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, r, g, b, 0xFF);
    uint32_t bgColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, 0, 0, 0, 0x00);

    // Clear surface to transparent
    SDL_memset(pixels, 0, static_cast<size_t>(w * h) * 4);

    // Render each character
    for (size_t ci = 0; ci < text.size(); ci++) {
        int ch = static_cast<unsigned char>(text[ci]);
        if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR) ch = ' ';
        int fontIdx = ch - FONT_FIRST_CHAR;

        for (int row = 0; row < FONT_CHAR_HEIGHT; row++) {
            uint8_t bits = FONT_DATA[fontIdx][row];
            for (int col = 0; col < FONT_CHAR_WIDTH; col++) {
                if (bits & (1 << (7 - col))) {
                    int px = static_cast<int>(ci) * FONT_CHAR_WIDTH + col;
                    int py = row;
                    if (px < w && py < h) {
                        pixels[py * pitch + px] = fgColor;
                    }
                }
            }
        }
    }

    SDL_UnlockSurface(surface);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_DestroySurface(surface);

    // Enable alpha blending
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    return texture;
}

SDL_Texture* TextRenderer::GetText(const std::string& text, uint8_t r, uint8_t g, uint8_t b) {
    // Build cache key
    char keyBuf[256];
    int n = SDL_snprintf(keyBuf, sizeof(keyBuf), "%s|%02x%02x%02x", text.c_str(), r, g, b);
    std::string key(keyBuf, static_cast<size_t>(n));

    // Check cache
    for (int i = 0; i < m_cacheCount; i++) {
        if (m_cache[i].key == key) {
            return m_cache[i].texture;
        }
    }

    // Create new texture
    SDL_Texture* tex = CreateTexture(text, r, g, b);
    if (!tex) return nullptr;

    // Evict oldest if cache full
    if (m_cacheCount >= MAX_CACHE) {
        if (m_cache[0].texture) SDL_DestroyTexture(m_cache[0].texture);
        // Shift remaining
        for (int i = 1; i < m_cacheCount; i++) {
            m_cache[i - 1] = m_cache[i];
        }
        m_cacheCount--;
    }

    m_cache[m_cacheCount].texture = tex;
    m_cache[m_cacheCount].key = key;
    m_cacheCount++;

    return tex;
}

void TextRenderer::DrawText(SDL_Texture* texture, float x, float y) {
    if (!texture || !m_renderer) return;

    float fw, fh;
    SDL_GetTextureSize(texture, &fw, &fh);
    SDL_FRect dest{x, y, fw, fh};
    SDL_RenderTexture(m_renderer, texture, nullptr, &dest);
}

void TextRenderer::RenderString(const std::string& text, float x, float y, uint8_t r, uint8_t g, uint8_t b) {
    SDL_Texture* tex = CreateTexture(text, r, g, b);
    if (tex) {
        DrawText(tex, x, y);
        SDL_DestroyTexture(tex);
    }
}

} // namespace engine::renderer
