#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace engine::renderer {

/// Simple bitmap text renderer using embedded 8x13 font.
/// Renders text to SDL_Texture, supports caching.
class TextRenderer {
public:
    explicit TextRenderer(SDL_Renderer* renderer);
    ~TextRenderer();

    /// Render text to a texture with given color.
    /// Returns a cached texture (do not free).
    SDL_Texture* GetText(const std::string& text, uint8_t r, uint8_t g, uint8_t b);

    /// Draw a text texture at screen position.
    void DrawText(SDL_Texture* texture, float x, float y);

    /// Calculate text pixel dimensions
    static int TextWidth(const std::string& text) { return static_cast<int>(text.size()) * 8; }
    static constexpr int TextHeight() { return 13; }

    /// Clear the texture cache
    void ClearCache();

    /// One-shot: render a string to screen at position with color.
    /// Creates temporary texture, draws, frees. Use for infrequent text.
    void RenderString(const std::string& text, float x, float y, uint8_t r, uint8_t g, uint8_t b);

private:
    struct CachedText {
        SDL_Texture* texture = nullptr;
        std::string key;
    };

    SDL_Renderer* m_renderer;
    static constexpr int MAX_CACHE = 32;
    CachedText m_cache[MAX_CACHE];
    int m_cacheCount = 0;

    SDL_Texture* CreateTexture(const std::string& text, uint8_t r, uint8_t g, uint8_t b);
};

} // namespace engine::renderer
