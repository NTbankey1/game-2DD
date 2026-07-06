#include "Application.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include <SDL3/SDL.h>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace engine::application {

static constexpr float W = 1280.0f;
static constexpr float H = 720.0f;
static constexpr float GROUND_Y = 680.0f;

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

static uint8_t LerpU8(uint8_t a, uint8_t b, float t) {
    return static_cast<uint8_t>(Lerp(static_cast<float>(a), static_cast<float>(b), t));
}

void Application::GenerateStars() {
    std::srand(42);
    for (int i = 0; i < STAR_COUNT; i++) {
        m_stars[i].x = static_cast<float>(std::rand() % 1280);
        m_stars[i].y = static_cast<float>(std::rand() % 600);
        m_stars[i].phase = static_cast<float>(std::rand() % 1000) / 1000.0f * 6.2832f;
        m_stars[i].baseBrightness = static_cast<uint8_t>(100 + std::rand() % 80);
    }
}

void Application::DrawStars(float brightnessScale) {
    float screen_scroll_x = m_bgOffset1;
    for (int i = 0; i < STAR_COUNT; i++) {
        float twinkle = 0.7f + 0.3f * std::sin(m_menuTimer * 2.0f + m_stars[i].phase);
        float bright = static_cast<float>(m_stars[i].baseBrightness) * (0.15f + 0.85f * twinkle) * brightnessScale;
        uint8_t b = static_cast<uint8_t>(std::min(bright, 255.0f));

        float sx = std::fmod(m_stars[i].x - screen_scroll_x * 0.05f, 1280.0f);
        if (sx < 0) sx += 1280.0f;

        SDL_SetRenderDrawColor(m_renderer->Handle(), b, b, static_cast<uint8_t>(static_cast<float>(b) * 0.85f + 30.0f), 0xFF);
        SDL_RenderPoint(m_renderer->Handle(), sx, m_stars[i].y);
    }
}

void Application::DrawClouds() {
    SDL_Renderer* r = m_renderer->Handle();
    m_cloudOffset += 12.0f * m_frameTime;
    if (m_cloudOffset > 1400.0f) m_cloudOffset -= 1400.0f;

    struct CloudDef { float w, y, speed; uint8_t a; };
    static const CloudDef clouds[] = {
        {200, 80,  0.3f, 25},
        {160, 130, 0.5f, 20},
        {240, 60,  0.2f, 22},
        {140, 170, 0.6f, 16},
        {180, 100, 0.4f, 18},
    };

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (const auto& c : clouds) {
        float cx = std::fmod(static_cast<float>(static_cast<double>(-m_cloudOffset) * static_cast<double>(c.speed)),
                             static_cast<float>(c.w) + 80.0f);
        if (cx < -c.w) cx += 1400.0f;

        SDL_SetRenderDrawColor(r, 0x88, 0x99, 0xBB, c.a);
        SDL_FRect rect{cx, c.y, c.w, 4};
        SDL_RenderFillRect(r, &rect);
        SDL_FRect rect2{cx + 20, c.y - 3, c.w - 40, 6};
        SDL_RenderFillRect(r, &rect2);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

void Application::DrawGradientBackground(uint8_t r1, uint8_t g1, uint8_t b1,
                                          uint8_t r2, uint8_t g2, uint8_t b2) {
    constexpr int STRIP_HEIGHT = 4;
    for (int y = 0; y < static_cast<int>(H); y += STRIP_HEIGHT) {
        float t = static_cast<float>(y) / H;
        SDL_SetRenderDrawColor(m_renderer->Handle(),
            LerpU8(r1, r2, t), LerpU8(g1, g2, t), LerpU8(b1, b2, t), 0xFF);
        SDL_FRect rect{0, static_cast<float>(y), W, static_cast<float>(STRIP_HEIGHT)};
        SDL_RenderFillRect(m_renderer->Handle(), &rect);
    }
}

void Application::DrawParallaxMountains() {
    SDL_Renderer* r = m_renderer->Handle();
    float scroll = GetScrollSpeed();

    // Far mountains
    m_bgOffset1 += scroll * 0.08f * m_frameTime;
    if (m_bgOffset1 > 200.0f) m_bgOffset1 -= 200.0f;

    SDL_SetRenderDrawColor(r, 0x22, 0x2B, 0x48, 0xFF);
    for (int i = 0; i < 8; i++) {
        float mx = static_cast<float>(i) * 200.0f - m_bgOffset1;
        if (mx < -200.0f) mx += 1800.0f;
        float mh = 70.0f + static_cast<float>((i * 37 + 13) % 5) * 25.0f;
        float mw = 110.0f + static_cast<float>((i * 23 + 7) % 4) * 40.0f;
        for (int row = 0; row < static_cast<int>(mh); row++) {
            float halfW = (mw / 2.0f) * (1.0f - static_cast<float>(row) / mh);
            SDL_FRect rect{mx + mw / 2.0f - halfW,
                GROUND_Y - mh + static_cast<float>(row), halfW * 2.0f, 1.0f};
            SDL_RenderFillRect(r, &rect);
        }
    }

    // Mid mountains
    m_bgOffset2 += scroll * 0.18f * m_frameTime;
    if (m_bgOffset2 > 160.0f) m_bgOffset2 -= 160.0f;

    SDL_SetRenderDrawColor(r, 0x18, 0x28, 0x38, 0xFF);
    for (int i = 0; i < 7; i++) {
        float mx = static_cast<float>(i) * 220.0f - m_bgOffset2;
        if (mx < -220.0f) mx += 1800.0f;
        float mh = 60.0f + static_cast<float>((i * 41 + 19) % 5) * 22.0f;
        float mw = 130.0f + static_cast<float>((i * 29 + 17) % 4) * 30.0f;
        for (int row = 0; row < static_cast<int>(mh); row++) {
            float halfW = (mw / 2.0f) * (1.0f - static_cast<float>(row) / mh);
            SDL_FRect rect{mx + mw / 2.0f - halfW,
                GROUND_Y - mh + static_cast<float>(row), halfW * 2.0f, 1.0f};
            SDL_RenderFillRect(r, &rect);
        }
    }

    // Near mountains
    float offset3 = m_bgOffset2 * 1.6f;
    if (offset3 > 200.0f) offset3 -= 200.0f;

    SDL_SetRenderDrawColor(r, 0x14, 0x2A, 0x1E, 0xFF);
    for (int i = 0; i < 6; i++) {
        float mx = static_cast<float>(i) * 280.0f - offset3;
        if (mx < -280.0f) mx += 1900.0f;
        float mh = 40.0f + static_cast<float>((i * 53 + 27) % 6) * 18.0f;
        float mw = 90.0f + static_cast<float>((i * 31 + 11) % 5) * 30.0f;
        for (int row = 0; row < static_cast<int>(mh); row++) {
            float halfW = (mw / 2.0f) * (1.0f - static_cast<float>(row) / mh);
            SDL_FRect rect{mx + mw / 2.0f - halfW,
                GROUND_Y - mh + static_cast<float>(row), halfW * 2.0f, 1.0f};
            SDL_RenderFillRect(r, &rect);
        }
    }
}

void Application::DrawSemiTransparentOverlay() {
    SDL_SetRenderDrawBlendMode(m_renderer->Handle(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(m_renderer->Handle(), 0x00, 0x00, 0x00, 0x88);
    SDL_FRect full{0, 0, W, H};
    SDL_RenderFillRect(m_renderer->Handle(), &full);
    SDL_SetRenderDrawBlendMode(m_renderer->Handle(), SDL_BLENDMODE_NONE);
}

void Application::DrawRoundedPanel(float x, float y, float w, float h,
                                    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_Renderer* rdr = m_renderer->Handle();
    SDL_SetRenderDrawBlendMode(rdr, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rdr, r, g, b, a);
    SDL_FRect rect{x, y, w, h};
    SDL_RenderFillRect(rdr, &rect);
    SDL_SetRenderDrawColor(rdr,
        static_cast<uint8_t>(std::min(static_cast<int>(r) + 30, 255)),
        static_cast<uint8_t>(std::min(static_cast<int>(g) + 30, 255)),
        static_cast<uint8_t>(std::min(static_cast<int>(b) + 30, 255)),
        static_cast<uint8_t>(static_cast<int>(a) + 30));
    SDL_FRect border{x, y, w, 2};
    SDL_RenderFillRect(rdr, &border);
    SDL_SetRenderDrawBlendMode(rdr, SDL_BLENDMODE_NONE);
}

void Application::CacheStaticTexts() {
    m_titleText = m_text->GetText("ENDLESS RUNNER", 0xFF, 0xCC, 0x33);
    m_promptText = m_text->GetText("Press ENTER to start", 0xAA, 0xBB, 0xFF);
    m_gameOverText = m_text->GetText("GAME OVER", 0xFF, 0x55, 0x55);
    m_pausedText = m_text->GetText("- PAUSED -", 0xFF, 0xFF, 0xAA);
}

} // namespace engine::application
