#include "Application.hpp"
#include "engine/platform/sdl3/SDLWindow.hpp"
#include "engine/platform/sdl3/SDLInputDevice.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/renderer/RenderSystem.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/physics/PhysicsSystem.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include "engine/physics/CollisionSystem.hpp"
#include "game/states/ExplorationState.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <spdlog/spdlog.h>
#include <string>
#include <format>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace engine::application {

// Texture creation (defined in ApplicationTextures.cpp)
SDL_Texture* CreatePlayerSheetTexture(SDL_Renderer* renderer);
SDL_Texture* CreateGroundTexture(SDL_Renderer* renderer);
SDL_Texture* CreateObstacleTexture(SDL_Renderer* renderer);
SDL_Texture* CreateCoinTexture(SDL_Renderer* renderer);

static constexpr float W = 1280.0f;
static constexpr float H = 720.0f;
static constexpr float GROUND_Y = 680.0f;

Application::Application()
    : m_stateMachine(std::make_unique<engine::scene::GameStateMachine>()) {}

Application::~Application() { Shutdown(); }

// ============ GAME PARAMS ============

float Application::GetScrollSpeed() const {
    float t = static_cast<float>(m_score) / 500.0f;
    return 350.0f + (750.0f - 350.0f) * std::min(t, 1.0f);
}

float Application::GetSpawnInterval() const {
    float t = static_cast<float>(m_score) / 500.0f;
    return 2.0f - (2.0f - 0.5f) * std::min(t, 1.0f);
}

// ============ SPAWNING ============

void Application::SpawnPlayer() {
    if (!m_playerSheetTexture) {
        m_playerSheetTexture = CreatePlayerSheetTexture(m_renderer->Handle());
    }
    if (!m_playerSheetTexture) return;
    auto player = m_registry.create();
    m_registry.emplace<engine::renderer::TransformComponent>(player,
        core::Vec2f(100.0f, 0.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    m_registry.emplace<engine::renderer::SpriteComponent>(player,
        m_playerSheetTexture, core::Rectf(0, 0, 40, 54), 1);
    m_registry.emplace<engine::physics::VelocityComponent>(player);
    m_registry.emplace<engine::physics::GravityComponent>(player, 980.0f);
    m_registry.emplace<engine::physics::AABBComponent>(player, core::Rectf(4, 0, 32, 54));
    m_registry.emplace<engine::physics::PlayerTag>(player);
    m_registry.emplace<engine::physics::PlayerStateComponent>(player);
    m_playerFrame = 0;
    m_animTimer = 0.0f;
}

void Application::SpawnGround() {
    if (!m_groundTexture) {
        m_groundTexture = CreateGroundTexture(m_renderer->Handle());
    }
    if (!m_groundTexture) return;
    auto ground = m_registry.create();
    m_registry.emplace<engine::renderer::TransformComponent>(ground,
        core::Vec2f(0.0f, GROUND_Y), 0.0f, core::Vec2f(1.0f, 1.0f));
    m_registry.emplace<engine::renderer::SpriteComponent>(ground,
        m_groundTexture, core::Rectf(0, 0, 1280, 40), 0);
    m_registry.emplace<engine::physics::AABBComponent>(ground, core::Rectf(0, 0, 1280, 40));
    m_registry.emplace<engine::physics::GroundTag>(ground);
}

void Application::SpawnCoin() {
    if (!m_coinTexture) {
        m_coinTexture = CreateCoinTexture(m_renderer->Handle());
    }
    if (m_spawnTimer > 0.1f && (std::rand() % 100) < 3) {
        auto coin = m_registry.create();
        float y = 200.0f + static_cast<float>(std::rand() % 400);
        m_registry.emplace<engine::renderer::TransformComponent>(coin,
            core::Vec2f(1280.0f, y), 0.0f, core::Vec2f(1.0f, 1.0f));
        m_registry.emplace<engine::renderer::SpriteComponent>(coin,
            m_coinTexture, core::Rectf(0, 0, 16, 16), 3);
        m_registry.emplace<engine::physics::AABBComponent>(coin, core::Rectf(0, 0, 16, 16));
        m_registry.emplace<CoinTag>(coin);
    }
}

void Application::ResetGame() {
    m_registry.clear();
    m_score = 0;
    m_spawnTimer = 0.0f;
    m_coins = 0;
    m_combo = 0;
    m_distance = 0.0f;
    m_phase = 1;
    m_debounceEnter = true;
    m_gameOver = false;
    m_bgOffset1 = 0.0f;
    m_bgOffset2 = 0.0f;
    m_playerAnim = PlayerAnimState{};
    m_playerAnim.wasAlive = true;
    m_shakeAmount = 0.0f;
    for (int i = 0; i < MAX_TRAIL; i++) m_trail[i].life = 0.0f;
    for (int i = 0; i < MAX_POPUPS; i++) m_popups[i].active = false;
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
    if (!TTF_Init()) {
        spdlog::error("TTF_Init failed: {}", SDL_GetError());
        return false;
    }
    m_text = std::make_unique<engine::renderer::TextRenderer>(m_renderer->Handle());
    m_text->Initialize(std::string(FONT_DIR) + "/font.ttf");
    m_camera.SetViewport(core::Vec2f(1280.0f, 720.0f));
    m_obstacleTexture = CreateObstacleTexture(m_renderer->Handle());

    GenerateStars();
    CacheStaticTexts();
    SDL_SetWindowTitle(m_window->Handle(), "Endless Runner");

    m_running = true;
    return true;
}

// ============ RUN ============

void Application::Run() {
    if (!m_running) return;
    spdlog::info("Application::Run started");

    std::uint64_t prevCounter = SDL_GetPerformanceCounter();
    const std::uint64_t counterFreq = SDL_GetPerformanceFrequency();

    while (m_running) {
        m_input->PollEvents();
        if (m_input->QuitRequested()) { m_running = false; break; }

        std::uint64_t currCounter = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(currCounter - prevCounter) / static_cast<float>(counterFreq);
        prevCounter = currCounter;
        if (deltaTime > 0.25f) deltaTime = 0.25f;
        m_frameTime = deltaTime;
        m_menuTimer += deltaTime;

        bool confirmNow = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RETURN];
        if (confirmNow && m_debounceEnter) { confirmNow = false; }
        else if (!confirmNow) { m_debounceEnter = false; }

        if (m_appState == AppState::Menu) {
            if (confirmNow) {
                m_appState = AppState::Playing;
                // Clear any stale states from stack
                while (!m_stateMachine->Empty())
                    m_stateMachine->PopScene();
                m_stateMachine->PushScene(std::make_unique<game::ExplorationState>(*this));
                continue;
            }

            m_renderer->BeginFrame();
            DrawGradientBackground(0x08, 0x08, 0x30, 0x18, 0x10, 0x40);
            DrawStars(1.0f);
            DrawClouds();
            DrawParallaxMountains();

            // Title with glow effect
            m_text->RenderString("ENDLESS RUNNER", 390, 190, 0xFF, 0xCC, 0x33, 36);
            m_text->RenderString("ENDLESS RUNNER", 391, 191, 0x88, 0x66, 0x00, 36); // shadow

            float pulse = 0.6f + 0.4f * std::sin(m_menuTimer * 2.5f);
            uint8_t pr = static_cast<uint8_t>(core::Lerp(85.0f, 204.0f, pulse));
            uint8_t pg = static_cast<uint8_t>(core::Lerp(119.0f, 221.0f, pulse));
            uint8_t pb = static_cast<uint8_t>(core::Lerp(187.0f, 255.0f, pulse));
            float pw = static_cast<float>(m_text->TextWidth("Press ENTER to start", 22));
            m_text->RenderString("Press ENTER to start", (1280.0f - pw) / 2.0f, 350.0f, pr, pg, pb, 22);

            m_text->RenderString("SPACE / UP / W to jump  |  ESC to pause", 450, 420, 0x66, 0x66, 0x99, 16);
            m_text->RenderString("v0.1.0", 610, 670, 0x55, 0x55, 0x88, 14);
            m_renderer->EndFrame();
            continue;
        }

        m_stateMachine->Update(deltaTime);
        m_stateMachine->FixedUpdate(m_frameTime);
        m_stateMachine->Render();
    }

    spdlog::info("Application::Run ended");
}

void Application::Shutdown() {
    if (!m_running && !m_window) return;
    if (m_playerSheetTexture) { SDL_DestroyTexture(m_playerSheetTexture); m_playerSheetTexture = nullptr; }
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
    TTF_Quit();
    SDL_Quit();
    m_running = false;
}

} // namespace engine::application
