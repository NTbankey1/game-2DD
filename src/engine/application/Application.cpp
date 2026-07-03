#include "Application.hpp"
#include "engine/platform/sdl3/SDLWindow.hpp"
#include "engine/platform/sdl3/SDLInputDevice.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/renderer/RenderSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/renderer/BitmapFont.hpp"
#include "engine/physics/PhysicsSystem.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include "engine/physics/CollisionSystem.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <string>
#include <format>
#include <cstdlib>
#include <cmath>

namespace engine::application {

static constexpr float BASE_SCROLL = 350.0f;
static constexpr float MAX_SCROLL = 750.0f;
static constexpr float BASE_INTERVAL = 2.0f;
static constexpr float MIN_INTERVAL = 0.5f;

// Mountain definitions for parallax background
struct Mountain {
    float x, height, width;
    uint8_t r, g, b;
};

static constexpr int NUM_FAR_MOUNTAINS = 6;
static constexpr int NUM_NEAR_MOUNTAINS = 4;

Application::Application()
    : m_stateMachine(std::make_unique<engine::scene::GameStateMachine>()) {}

Application::~Application() { Shutdown(); }

static SDL_Texture* CreateSolidTexture(SDL_Renderer* renderer, int w, int h,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_Surface* surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) return nullptr;
    SDL_FillSurfaceRect(surface, nullptr,
        SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, r, g, b, a));
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return tex;
}

static void SetPixel(SDL_Surface* surf, int x, int y, uint32_t color) {
    if (x < 0 || x >= surf->w || y < 0 || y >= surf->h) return;
    ((uint32_t*)surf->pixels)[y * (surf->pitch / 4) + x] = color;
}

static uint32_t RGBA(SDL_Surface* surf, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, r, g, b, a);
}

static SDL_Texture* CreatePlayerTexture(SDL_Renderer* renderer) {
    SDL_Surface* surf = SDL_CreateSurface(40, 50, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t skin = RGBA(surf, 0xFF, 0xCC, 0x88, 0xFF);
    uint32_t pants = RGBA(surf, 0x44, 0x88, 0xFF, 0xFF);
    uint32_t shoes = RGBA(surf, 0x66, 0x44, 0x22, 0xFF);
    uint32_t eye = RGBA(surf, 0xFF, 0xFF, 0xFF, 0xFF);
    uint32_t pupil = RGBA(surf, 0x22, 0x22, 0x22, 0xFF);

    // Head (circle-like, rows 0-12)
    for (int y = 1; y < 11; y++) {
        int halfW = (11 - y) * 2;
        if (halfW < 2) halfW = 2;
        for (int x = 20 - halfW; x < 20 + halfW; x++) {
            if (x >= 0 && x < 40) SetPixel(surf, x, y, skin);
        }
    }
    // Eyes (row 5-6)
    SetPixel(surf, 15, 5, eye); SetPixel(surf, 16, 5, eye);
    SetPixel(surf, 24, 5, eye); SetPixel(surf, 25, 5, eye);
    SetPixel(surf, 16, 6, pupil); SetPixel(surf, 24, 6, pupil);

    // Body (rows 12-28)
    for (int y = 11; y < 28; y++) {
        for (int x = 12; x < 28; x++) {
            SetPixel(surf, x, y, pants);
        }
    }

    // Arms (rows 12-24, left and right)
    for (int y = 14; y < 24; y++) {
        for (int x = 6; x < 12; x++) SetPixel(surf, x, y, skin);
        for (int x = 28; x < 34; x++) SetPixel(surf, x, y, skin);
    }

    // Legs (rows 28-45)
    for (int y = 28; y < 42; y++) {
        for (int x = 13; x < 19; x++) SetPixel(surf, x, y, pants);
        for (int x = 21; x < 27; x++) SetPixel(surf, x, y, pants);
    }

    // Shoes
    for (int y = 42; y < 47; y++) {
        for (int x = 11; x < 20; x++) SetPixel(surf, x, y, shoes);
        for (int x = 20; x < 29; x++) SetPixel(surf, x, y, shoes);
    }

    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

static SDL_Texture* CreateGroundTexture(SDL_Renderer* renderer) {
    SDL_Surface* surf = SDL_CreateSurface(1280, 40, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t grass = RGBA(surf, 0x44, 0xBB, 0x44, 0xFF);
    uint32_t darkGrass = RGBA(surf, 0x33, 0x99, 0x33, 0xFF);
    uint32_t dirt = RGBA(surf, 0x8B, 0x6B, 0x4A, 0xFF);
    uint32_t darkDirt = RGBA(surf, 0x6B, 0x4A, 0x2A, 0xFF);

    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 1280; x++) {
            if (y < 5) {
                // Grass top — alternating grass blades
                uint32_t c = ((x + y * 3) % 4 < 2) ? grass : darkGrass;
                SetPixel(surf, x, y, c);
            } else {
                // Dirt with variation
                uint32_t c = ((x + y * 7) % 6 < 4) ? dirt : darkDirt;
                SetPixel(surf, x, y, c);
            }
        }
    }
    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

static SDL_Texture* CreateObstacleTexture(SDL_Renderer* renderer) {
    // A spike/cactus shape
    SDL_Surface* surf = SDL_CreateSurface(36, 50, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t body = RGBA(surf, 0xDD, 0x55, 0x00, 0xFF);
    uint32_t dark = RGBA(surf, 0xAA, 0x33, 0x00, 0xFF);
    uint32_t spike = RGBA(surf, 0xFF, 0x77, 0x22, 0xFF);

    for (int y = 0; y < 50; y++) {
        int halfW = 4 + (50 - y) / 3;
        if (halfW < 2) halfW = 2;
        for (int x = 18 - halfW; x < 18 + halfW; x++) {
            if (x < 0 || x >= 36) continue;
            uint32_t c = ((x + y) % 3 == 0) ? dark : body;
            SetPixel(surf, x, y, c);
        }
    }
    // Spikes on sides
    for (int y = 5; y < 40; y += 8) {
        int xL = 18 - 4 - (50 - y) / 3;
        int xR = 18 + 4 + (50 - y) / 3;
        for (int dy = -2; dy <= 2; dy++) {
            if (xL > 0) SetPixel(surf, xL, y + dy, spike);
            if (xR < 36) SetPixel(surf, xR, y + dy, spike);
        }
    }
    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

static SDL_Texture* CreateCoinTexture(SDL_Renderer* renderer) {
    SDL_Surface* surf = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_RGBA8888);
    if (!surf) return nullptr;
    SDL_LockSurface(surf);
    uint32_t gold = RGBA(surf, 0xFF, 0xDD, 0x00, 0xFF);
    uint32_t bright = RGBA(surf, 0xFF, 0xEE, 0x66, 0xFF);
    uint32_t dark = RGBA(surf, 0xAA, 0x88, 0x00, 0xFF);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int dx = x - 8, dy = y - 8;
            int dist2 = dx * dx + dy * dy;
            if (dist2 > 56) continue;  // outside circle
            uint32_t c;
            if (dist2 < 20) c = bright;
            else if (dist2 < 36) c = gold;
            else c = dark;
            SetPixel(surf, x, y, c);
        }
    }
    SDL_UnlockSurface(surf);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}

float Application::GetScrollSpeed() const {
    float t = static_cast<float>(m_score) / 500.0f;
    return BASE_SCROLL + (MAX_SCROLL - BASE_SCROLL) * std::min(t, 1.0f);
}

float Application::GetSpawnInterval() const {
    float t = static_cast<float>(m_score) / 500.0f;
    return BASE_INTERVAL - (BASE_INTERVAL - MIN_INTERVAL) * std::min(t, 1.0f);
}

// ============ DRAWING HELPERS ============

static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

static uint8_t LerpU8(uint8_t a, uint8_t b, float t) {
    return static_cast<uint8_t>(Lerp(static_cast<float>(a), static_cast<float>(b), t));
}

void Application::DrawGradientBackground(uint8_t r1, uint8_t g1, uint8_t b1,
                                          uint8_t r2, uint8_t g2, uint8_t b2) {
    constexpr int STRIP_HEIGHT = 8;
    for (int y = 0; y < 720; y += STRIP_HEIGHT) {
        float t = static_cast<float>(y) / 720.0f;
        uint8_t r = LerpU8(r1, r2, t);
        uint8_t g = LerpU8(g1, g2, t);
        uint8_t b = LerpU8(b1, b2, t);
        SDL_SetRenderDrawColor(m_renderer->Handle(), r, g, b, 0xFF);
        SDL_FRect rect{0, static_cast<float>(y), 1280.0f, static_cast<float>(STRIP_HEIGHT)};
        SDL_RenderFillRect(m_renderer->Handle(), &rect);
    }
}

void Application::DrawParallaxMountains() {
    SDL_Renderer* r = m_renderer->Handle();
    float scroll = GetScrollSpeed();

    // Far mountains (slow parallax) — dark blue-gray
    m_bgOffset1 += scroll * 0.1f * m_frameTime;
    if (m_bgOffset1 > 200.0f) m_bgOffset1 -= 200.0f;

    for (int i = 0; i < 8; i++) {
        float mx = static_cast<float>(i) * 200.0f - m_bgOffset1;
        if (mx < -200.0f) mx += 1800.0f;
        float mh = 80.0f + static_cast<float>((i * 37 + 13) % 5) * 30.0f;
        float mw = 120.0f + static_cast<float>((i * 23 + 7) % 4) * 40.0f;
        SDL_SetRenderDrawColor(r, 0x25, 0x30, 0x50, 0xFF);
        for (int row = 0; row < static_cast<int>(mh); row++) {
            float halfW = (mw / 2.0f) * (1.0f - static_cast<float>(row) / mh);
            SDL_FRect rect{mx + mw / 2.0f - halfW, 680.0f - mh + static_cast<float>(row), halfW * 2.0f, 1.0f};
            SDL_RenderFillRect(r, &rect);
        }
    }

    // Near mountains (faster parallax) — dark green-gray
    m_bgOffset2 += scroll * 0.25f * m_frameTime;
    if (m_bgOffset2 > 160.0f) m_bgOffset2 -= 160.0f;

    for (int i = 0; i < 6; i++) {
        float mx = static_cast<float>(i) * 260.0f - m_bgOffset2;
        if (mx < -260.0f) mx += 1800.0f;
        float mh = 50.0f + static_cast<float>((i * 53 + 27) % 6) * 20.0f;
        float mw = 100.0f + static_cast<float>((i * 31 + 11) % 5) * 30.0f;
        SDL_SetRenderDrawColor(r, 0x1A, 0x35, 0x2A, 0xFF);
        for (int row = 0; row < static_cast<int>(mh); row++) {
            float halfW = (mw / 2.0f) * (1.0f - static_cast<float>(row) / mh);
            SDL_FRect rect{mx + mw / 2.0f - halfW, 680.0f - mh + static_cast<float>(row), halfW * 2.0f, 1.0f};
            SDL_RenderFillRect(r, &rect);
        }
    }
}

void Application::DrawSemiTransparentOverlay() {
    SDL_SetRenderDrawColor(m_renderer->Handle(), 0x00, 0x00, 0x00, 0x80);
    SDL_FRect full{0, 0, 1280.0f, 720.0f};
    SDL_RenderFillRect(m_renderer->Handle(), &full);
}

void Application::DrawCenteredText(const std::string& text, float y, uint8_t r, uint8_t g, uint8_t b) {
    SDL_Texture* tex = m_text->GetText(text, r, g, b);
    if (!tex) return;
    float fw;
    SDL_GetTextureSize(tex, &fw, nullptr);
    float x = (1280.0f - fw) / 2.0f;
    m_text->DrawText(tex, x, y);
}

void Application::CacheStaticTexts() {
    m_titleText = m_text->GetText("ENDLESS RUNNER", 0xFF, 0xCC, 0x33);
    m_promptText = m_text->GetText("Press ENTER to start", 0xAA, 0xBB, 0xFF);
    m_gameOverText = m_text->GetText("GAME OVER", 0xFF, 0x44, 0x44);
    m_pausedText = m_text->GetText("- PAUSED -", 0xFF, 0xFF, 0xAA);
}

// ============ SPAWNING ============

void Application::SpawnPlayer() {
    if (!m_playerTexture) {
        m_playerTexture = CreatePlayerTexture(m_renderer->Handle());
    }
    if (!m_playerTexture) return;
    auto player = m_registry.create();
    m_registry.emplace<engine::renderer::TransformComponent>(player,
        core::Vec2f(100.0f, 0.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    m_registry.emplace<engine::renderer::SpriteComponent>(player,
        m_playerTexture, core::Rectf(0, 0, 40, 50), 1);
    m_registry.emplace<engine::physics::VelocityComponent>(player);
    m_registry.emplace<engine::physics::GravityComponent>(player, 980.0f);
    m_registry.emplace<engine::physics::AABBComponent>(player, core::Rectf(0, 0, 40, 50));
    m_registry.emplace<engine::physics::PlayerTag>(player);
    m_registry.emplace<engine::physics::PlayerStateComponent>(player);
}

void Application::SpawnGround() {
    if (!m_groundTexture) {
        m_groundTexture = CreateGroundTexture(m_renderer->Handle());
    }
    if (!m_groundTexture) return;
    auto ground = m_registry.create();
    m_registry.emplace<engine::renderer::TransformComponent>(ground,
        core::Vec2f(0.0f, 680.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    m_registry.emplace<engine::renderer::SpriteComponent>(ground,
        m_groundTexture, core::Rectf(0, 0, 1280, 40), 0);
    m_registry.emplace<engine::physics::AABBComponent>(ground, core::Rectf(0, 0, 1280, 40));
    m_registry.emplace<engine::physics::GroundTag>(ground);
}

void Application::SpawnCoin() {
    if (!m_coinTexture) {
        m_coinTexture = CreateCoinTexture(m_renderer->Handle());
    }
    if (m_spawnTimer > 0.1f && (rand() % 100) < 3) {
        auto coin = m_registry.create();
        float y = 200.0f + static_cast<float>(rand() % 400);
        m_registry.emplace<engine::renderer::TransformComponent>(coin,
            core::Vec2f(1280.0f, y), 0.0f, core::Vec2f(1.0f, 1.0f));
        m_registry.emplace<engine::renderer::SpriteComponent>(coin,
            m_coinTexture, core::Rectf(0, 0, 16, 16), 3);
    }
}

void Application::ResetGame() {
    m_registry.clear();
    m_score = 0;
    m_spawnTimer = 0.0f;
    m_debounceEnter = true;
    m_gameOver = false;
    m_bgOffset1 = 0.0f;
    m_bgOffset2 = 0.0f;
    m_appState = AppState::Playing;
    SpawnPlayer();
    SpawnGround();
}

// ============ INITIALIZE ============

bool Application::Initialize() {
    spdlog::info("Application::Initialize");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    m_window.reset(engine::platform::sdl3::SDLWindow::Create("Endless Runner", 1280, 720));
    if (!m_window) return false;
    m_input = std::make_unique<engine::platform::sdl3::SDLInputDevice>();
    m_renderer = std::make_unique<engine::platform::sdl3::SDLRenderer>(*m_window);
    if (!m_renderer->Initialize()) return false;
    m_renderSystem = std::make_unique<engine::renderer::RenderSystem>(*m_renderer);
    m_text = std::make_unique<engine::renderer::TextRenderer>(m_renderer->Handle());
    m_camera.SetViewport(core::Vec2f(1280.0f, 720.0f));
    m_obstacleTexture = CreateObstacleTexture(m_renderer->Handle());

    CacheStaticTexts();
    SDL_SetWindowTitle(m_window->Handle(), "Endless Runner");

    m_running = true;
    return true;
}

// ============ RUN ============

void Application::Run() {
    if (!m_running) return;
    spdlog::info("Application::Run started");

    engine::physics::PhysicsSystem physics;
    engine::physics::CollisionSystem collision;

    float accumulator = 0.0f;
    std::uint64_t prevCounter = SDL_GetPerformanceCounter();
    const std::uint64_t counterFreq = SDL_GetPerformanceFrequency();

    // Cached score/fps text textures
    SDL_Texture* scoreTex = nullptr;

    while (m_running) {
        m_input->PollEvents();
        if (m_input->QuitRequested()) { m_running = false; break; }

        std::uint64_t currCounter = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(currCounter - prevCounter) / static_cast<float>(counterFreq);
        prevCounter = currCounter;
        if (deltaTime > 0.25f) deltaTime = 0.25f;
        m_frameTime = deltaTime;

        bool confirmNow = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RETURN];
        if (confirmNow && m_debounceEnter) { confirmNow = false; }
        else if (!confirmNow) { m_debounceEnter = false; }

        // ====== MENU STATE ======
        if (m_appState == AppState::Menu) {
            if (confirmNow) { ResetGame(); continue; }

            m_renderer->BeginFrame();
            DrawGradientBackground(0x10, 0x10, 0x40, 0x08, 0x08, 0x20);

            // Title with glow effect (drawn twice for glow look)
            float titleY = 200.0f;
            m_text->DrawText(m_titleText, 639, titleY - 1);
            m_text->DrawText(m_titleText, 641, titleY + 1);
            m_text->DrawText(m_titleText, 640, titleY);

            float promptY = 350.0f;
            // Draw prompt with subtle fade
            m_text->DrawText(m_promptText, 640, promptY);

            // Bottom info
            m_text->RenderString("v0.1.0", 590, 650, 0x66, 0x66, 0x99);
            m_renderer->EndFrame();
            continue;
        }

        // ====== GAME OVER STATE ======
        if (m_appState == AppState::GameOver) {
            if (m_score > m_highScore) m_highScore = m_score;
            if (confirmNow) { ResetGame(); continue; }

            m_renderer->BeginFrame();
            DrawGradientBackground(0x30, 0x08, 0x08, 0x18, 0x04, 0x04);
            DrawParallaxMountains();
            // Draw the frozen game scene dimmed
            // (We draw it, then overlay for dimming)
            m_renderSystem->Render(m_registry, m_camera);
            // But we need to clear the depth - simpler: skip game scene render
            // Re-draw background over everything
            DrawGradientBackground(0x30, 0x08, 0x08, 0x18, 0x04, 0x04);

            // GAME OVER title
            float gy = 180.0f;
            m_text->DrawText(m_gameOverText, 640, gy - 1);
            m_text->DrawText(m_gameOverText, 640, gy);

            // Score
            std::string scoreStr = std::format("Score: {}", m_score);
            m_text->RenderString(scoreStr.c_str(), 640 - static_cast<float>(scoreStr.size()) * 4.0f, 260, 0xFF, 0xCC, 0xCC);

            std::string bestStr = std::format("Best: {}", m_highScore);
            m_text->RenderString(bestStr.c_str(), 640 - static_cast<float>(bestStr.size()) * 4.0f, 290, 0xFF, 0x88, 0x88);

            m_text->RenderString("Press ENTER to restart", 640 - 70, 400, 0xAA, 0xAA, 0xFF);

            m_renderer->EndFrame();
            continue;
        }

        // ====== PAUSED STATE ======
        if (m_appState == AppState::Paused) {
            if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE]) {
                m_appState = AppState::Playing;
            }

            m_renderer->BeginFrame();
            DrawGradientBackground(0x15, 0x18, 0x25, 0x0C, 0x0E, 0x18);
            DrawParallaxMountains();
            m_renderSystem->Render(m_registry, m_camera);
            DrawSemiTransparentOverlay();

            // Paused text
            float py = 300.0f;
            m_text->DrawText(m_pausedText, 640, py - 1);
            m_text->DrawText(m_pausedText, 640, py);

            m_text->RenderString("ESC to resume", 640 - 50, 350, 0xAA, 0xAA, 0x88);
            m_renderer->EndFrame();
            continue;
        }

        // ====== PLAYING STATE ======
        float scrollSpeed = GetScrollSpeed();
        float spawnInterval = GetSpawnInterval();
        bool jumpHeld = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_SPACE]
                     || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_UP]
                     || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W];

        if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE]) {
            m_appState = AppState::Paused; continue;
        }

        // Fixed timestep physics
        accumulator += deltaTime;
        m_gameOver = false;
        while (accumulator >= FIXED_DT) {
            physics.FixedUpdate(m_registry, FIXED_DT);
            collision.FixedUpdate(m_registry, m_eventBus, m_gameOver, m_score);
            accumulator -= FIXED_DT;
        }
        if (m_gameOver) { m_appState = AppState::GameOver; continue; }

        // ====== COYOTE TIME + JUMP BUFFER ======
        if (jumpHeld) {
            for (auto e : m_registry.view<engine::physics::PlayerStateComponent>()) {
                auto& ps = m_registry.get<engine::physics::PlayerStateComponent>(e);
                if (ps.jumpBufferTimer == 0.0f) ps.jumpBufferTimer = 0.001f;
            }
        }

        constexpr float COYOTE_MS = 0.07f;
        constexpr float BUFFER_MS = 0.10f;
        constexpr float JUMP_V = -500.0f;

        for (auto e : m_registry.view<engine::physics::PlayerStateComponent>()) {
            auto& ps = m_registry.get<engine::physics::PlayerStateComponent>(e);
            ps.jumpHeld = jumpHeld;

            bool canJump = (ps.isGrounded || ps.coyoteTimer < COYOTE_MS)
                        && ps.jumpBufferTimer > 0.0f
                        && ps.jumpBufferTimer < BUFFER_MS;

            if (canJump) {
                if (m_registry.all_of<engine::physics::VelocityComponent>(e)) {
                    auto& vel = m_registry.get<engine::physics::VelocityComponent>(e);
                    vel.velocity.y = JUMP_V;
                    ps.isGrounded = false;
                    ps.coyoteTimer = 99.0f;
                }
                ps.jumpBufferTimer = 0.0f;
            }

            if (ps.jumpBufferTimer > BUFFER_MS) ps.jumpBufferTimer = 0.0f;
        }

        // Keep player at fixed X
        for (auto e : m_registry.view<engine::physics::PlayerTag>()) {
            auto& pos = m_registry.get<engine::renderer::TransformComponent>(e);
            pos.position.x = 100.0f;
        }

        // Spawn obstacles
        m_spawnTimer += deltaTime;
        if (m_spawnTimer >= spawnInterval) {
            m_spawnTimer -= spawnInterval;
            if (m_obstacleTexture) {
                auto obs = m_registry.create();
                m_registry.emplace<engine::renderer::TransformComponent>(obs,
                    core::Vec2f(1280.0f, 630.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
                m_registry.emplace<engine::renderer::SpriteComponent>(obs,
                    m_obstacleTexture, core::Rectf(0, 0, 36, 50), 2);
                m_registry.emplace<engine::physics::AABBComponent>(obs,
                    core::Rectf(0, 0, 36, 50));
            }
            SpawnCoin();
        }

        // Scroll off-screen entities
        std::vector<entt::entity> toRemove;
        auto scrollView = m_registry.view<engine::renderer::TransformComponent>();
        for (auto e : scrollView) {
            if (m_registry.all_of<engine::physics::PlayerTag>(e)) continue;
            if (m_registry.all_of<engine::physics::GroundTag>(e)) continue;
            auto& pos = scrollView.get<engine::renderer::TransformComponent>(e);
            pos.position.x -= scrollSpeed * deltaTime;
            if (pos.position.x < -100.0f) toRemove.push_back(e);
        }
        for (auto e : toRemove) m_registry.destroy(e);

        // Score
        m_score += static_cast<int>(deltaTime * 10.0f);

        // ====== RENDER ======
        m_renderer->BeginFrame();

        // 1. Sky gradient
        DrawGradientBackground(0x30, 0x50, 0x80, 0x15, 0x25, 0x50);

        // 2. Parallax mountains
        DrawParallaxMountains();

        // 3. Game world (ECS entities)
        m_renderSystem->Render(m_registry, m_camera);

        // 4. HUD — score in top-left
        std::string scoreStr = std::format("Score: {}", m_score);
        SDL_Texture* st = m_text->GetText(scoreStr, 0xFF, 0xFF, 0xFF);
        if (st) m_text->DrawText(st, 10, 10);

        // Speed indicator top-right
        std::string spdStr = std::format("Speed: {:.0f}", scrollSpeed);
        SDL_Texture* spt = m_text->GetText(spdStr, 0xAA, 0xAA, 0xCC);
        if (spt) {
            float sw;
            SDL_GetTextureSize(spt, &sw, nullptr);
            m_text->DrawText(spt, 1270 - sw, 10);
        }

        m_renderer->EndFrame();
    }

    spdlog::info("Application::Run ended — final score: {}", m_score);
}

void Application::Shutdown() {
    if (!m_running && !m_window) return;
    if (m_playerTexture) { SDL_DestroyTexture(m_playerTexture); m_playerTexture = nullptr; }
    if (m_groundTexture) { SDL_DestroyTexture(m_groundTexture); m_groundTexture = nullptr; }
    if (m_obstacleTexture) { SDL_DestroyTexture(m_obstacleTexture); m_obstacleTexture = nullptr; }
    if (m_coinTexture) { SDL_DestroyTexture(m_coinTexture); m_coinTexture = nullptr; }
    m_registry.clear();
    m_text.reset();
    m_renderSystem.reset();
    m_renderer.reset();
    m_stateMachine.reset();
    m_input.reset();
    m_window.reset();
    SDL_Quit();
    m_running = false;
}

} // namespace engine::application
