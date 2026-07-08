#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

namespace engine::renderer {

/// TrueType-based text renderer using SDL3_ttf.
/// Supports multiple font sizes, color, and caching.
class TextRenderer {
public:
    explicit TextRenderer(SDL_Renderer* renderer);
    ~TextRenderer();

    /// Initialize with a TTF font file path (optional — defaults to assets/font.ttf)
    bool Initialize(const std::string& fontPath);

    /// Render text to a texture with given color and optional size.
    /// Returns a cached texture (do not free).
    /// Defaults to size=22 if not specified
    SDL_Texture* GetText(const std::string& text, uint8_t r, uint8_t g, uint8_t b, int size = 22);

    /// Draw a text texture at screen position.
    void DrawText(SDL_Texture* texture, float x, float y);

    /// One-shot: render string with color at position. Creates temp texture.
    void RenderString(const std::string& text, float x, float y, uint8_t r, uint8_t g, uint8_t b, int size = 22);

    /// Clear the texture cache
    void ClearCache();

    /// Calculate rendered width of a text string for given size
    int TextWidth(const std::string& text, int size = 22) const;

    /// Default font height
    static constexpr int TextHeight(int size = 22) { return size; }

private:
    struct CachedText {
        SDL_Texture* texture = nullptr;
        std::string key;
    };
    static constexpr int MAX_CACHE = 64;

    SDL_Renderer* m_renderer;
    TTF_Font* m_font20 = nullptr;  // small
    TTF_Font* m_font22 = nullptr;  // normal
    TTF_Font* m_font30 = nullptr;  // large
    CachedText m_cache[MAX_CACHE];
    int m_cacheCount = 0;

    SDL_Texture* CreateTexture(const std::string& text, uint8_t r, uint8_t g, uint8_t b, TTF_Font* font);
};

} // namespace engine::renderer
