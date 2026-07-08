#include "TextRenderer.hpp"
#include <SDL3/SDL.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <format>

namespace engine::renderer {

TextRenderer::TextRenderer(SDL_Renderer* renderer)
    : m_renderer(renderer) {
    for (int i = 0; i < MAX_CACHE; i++) m_cache[i].texture = nullptr;
}

TextRenderer::~TextRenderer() {
    ClearCache();
    if (m_font20) TTF_CloseFont(m_font20);
    if (m_font22) TTF_CloseFont(m_font22);
    if (m_font30) TTF_CloseFont(m_font30);
}

bool TextRenderer::Initialize(const std::string& fontPath) {
    m_font20 = TTF_OpenFont(fontPath.c_str(), 20.0f);
    m_font22 = TTF_OpenFont(fontPath.c_str(), 22.0f);
    m_font30 = TTF_OpenFont(fontPath.c_str(), 30.0f);
    if (!m_font22) {
        SDL_Log("TextRenderer: failed to open font: %s", SDL_GetError());
        return false;
    }
    return true;
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

SDL_Texture* TextRenderer::CreateTexture(const std::string& text, uint8_t r, uint8_t g, uint8_t b, TTF_Font* font) {
    if (text.empty() || !font) return nullptr;

    SDL_Color color{r, g, b, 0xFF};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.size(), color);
    if (!surface) return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_DestroySurface(surface);

    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    return texture;
}

SDL_Texture* TextRenderer::GetText(const std::string& text, uint8_t r, uint8_t g, uint8_t b, int size) {
    // Build cache key
    std::string key = text + "|" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + "," + std::to_string(size);

    // Check cache
    for (int i = 0; i < m_cacheCount; i++) {
        if (m_cache[i].key == key) return m_cache[i].texture;
    }

    // Pick font size
    TTF_Font* font = m_font22;
    if (size <= 20) font = m_font20;
    else if (size >= 30) font = m_font30;

    // Create texture
    SDL_Texture* tex = CreateTexture(text, r, g, b, font);
    if (!tex) return nullptr;

    // Evict oldest if full
    if (m_cacheCount >= MAX_CACHE) {
        if (m_cache[0].texture) SDL_DestroyTexture(m_cache[0].texture);
        for (int i = 1; i < m_cacheCount; i++) m_cache[i - 1] = m_cache[i];
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

void TextRenderer::RenderString(const std::string& text, float x, float y, uint8_t r, uint8_t g, uint8_t b, int size) {
    SDL_Texture* tex = GetText(text, r, g, b, size);
    if (tex) DrawText(tex, x, y);
}

int TextRenderer::TextWidth(const std::string& text, int size) const {
    if (text.empty()) return 0;
    TTF_Font* font = m_font22;
    if (size <= 20) font = m_font20;
    else if (size >= 30) font = m_font30;
    if (!font) return static_cast<int>(text.size()) * 8;
    int w = 0, h = 0;
    TTF_GetStringSize(font, text.c_str(), text.size(), &w, &h);
    return w;
}

} // namespace engine::renderer
