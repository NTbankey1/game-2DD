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
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <string>
#include <format>
#include <cstdlib>

namespace engine::application {

static constexpr float JUMP_FORCE = -520.0f;
static constexpr float BASE_SCROLL = 350.0f;
static constexpr float MAX_SCROLL = 750.0f;
static constexpr float BASE_INTERVAL = 2.0f;
static constexpr float MIN_INTERVAL = 0.5f;

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

float Application::GetScrollSpeed() const {
    float t = static_cast<float>(m_score) / 500.0f;
    return BASE_SCROLL + (MAX_SCROLL - BASE_SCROLL) * std::min(t, 1.0f);
}

float Application::GetSpawnInterval() const {
    float t = static_cast<float>(m_score) / 500.0f;
    return BASE_INTERVAL - (BASE_INTERVAL - MIN_INTERVAL) * std::min(t, 1.0f);
}

void Application::SpawnPlayer() {
    m_playerTexture = CreateSolidTexture(m_renderer->Handle(), 50, 50, 0xFF, 0x33, 0x33, 0xFF);
    if (!m_playerTexture) return;
    auto player = m_registry.create();
    m_registry.emplace<engine::renderer::TransformComponent>(player,
        core::Vec2f(100.0f, 0.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    m_registry.emplace<engine::renderer::SpriteComponent>(player,
        m_playerTexture, core::Rectf(0, 0, 50, 50), 1);
    m_registry.emplace<engine::physics::VelocityComponent>(player);
    m_registry.emplace<engine::physics::GravityComponent>(player, 980.0f);
    m_registry.emplace<engine::physics::AABBComponent>(player, core::Rectf(0, 0, 50, 50));
    m_registry.emplace<engine::physics::PlayerTag>(player);
}

void Application::SpawnGround() {
    m_groundTexture = CreateSolidTexture(m_renderer->Handle(), 1280, 40, 0x33, 0xCC, 0x33, 0xFF);
    if (!m_groundTexture) return;
    auto ground = m_registry.create();
    m_registry.emplace<engine::renderer::TransformComponent>(ground,
        core::Vec2f(0.0f, 680.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
    m_registry.emplace<engine::renderer::SpriteComponent>(ground,
        m_groundTexture, core::Rectf(0, 0, 1280, 40), 0);
    m_registry.emplace<engine::physics::AABBComponent>(ground, core::Rectf(0, 0, 1280, 40));
    m_registry.emplace<engine::physics::GroundTag>(ground);
}

void Application::SpawnCoins() {
    // Coins spawn periodically during gameplay
    if (m_spawnTimer > 0.1f && (rand() % 100) < 3) {
        if (!m_coinTexture) {
            m_coinTexture = CreateSolidTexture(m_renderer->Handle(), 12, 12, 0xFF, 0xDD, 0x00, 0xFF);
        }
        if (m_coinTexture) {
            auto coin = m_registry.create();
            float y = 200.0f + static_cast<float>(rand() % 400);
            m_registry.emplace<engine::renderer::TransformComponent>(coin,
                core::Vec2f(1280.0f, y), 0.0f, core::Vec2f(1.0f, 1.0f));
            m_registry.emplace<engine::renderer::SpriteComponent>(coin,
                m_coinTexture, core::Rectf(0, 0, 12, 12), 3);
        }
    }
}

void Application::ResetGame() {
    m_registry.clear();
    m_score = 0;
    m_spawnTimer = 0.0f;
    m_debounceEnter = true;
    m_appState = AppState::Playing;
    SpawnPlayer();
    SpawnGround();
}

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
    m_camera.SetViewport(core::Vec2f(1280.0f, 720.0f));
    m_obstacleTexture = CreateSolidTexture(m_renderer->Handle(), 40, 50, 0xFF, 0xCC, 0x00, 0xFF);
    m_coinTexture = CreateSolidTexture(m_renderer->Handle(), 12, 12, 0xFF, 0xDD, 0x00, 0xFF);
    SDL_SetWindowTitle(m_window->Handle(), "ENDLESS RUNNER — Press ENTER to start");
    m_running = true;
    return true;
}

void Application::Run() {
    if (!m_running) return;
    spdlog::info("Application::Run started");

    engine::physics::PhysicsSystem physics;
    engine::physics::CollisionSystem collision;

    float accumulator = 0.0f;
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

        bool confirmNow = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RETURN];
        bool escNow = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE];

        // Handle debounce for ENTER to prevent accidental double-fire
        if (confirmNow && m_debounceEnter) {
            confirmNow = false;
        } else if (!confirmNow) {
            m_debounceEnter = false;
        }

        // *** MENU STATE ***
        if (m_appState == AppState::Menu) {
            if (confirmNow) {
                ResetGame();
                m_renderer->BeginFrame();
                m_renderer->EndFrame();
                continue;
            }
            m_renderer->BeginFrame();
            SDL_SetRenderDrawColor(m_renderer->Handle(), 0x0A, 0x0A, 0x2A, 0xFF);
            SDL_RenderClear(m_renderer->Handle());
            m_renderer->EndFrame();
            continue;
        }

        // *** GAME OVER STATE ***
        if (m_appState == AppState::GameOver) {
            if (m_score > m_highScore) m_highScore = m_score;
            SDL_SetWindowTitle(m_window->Handle(),
                std::format("GAME OVER — Score: {} — Best: {} — ENTER to restart",
                    m_score, m_highScore).c_str());
            if (confirmNow) { ResetGame(); continue; }
            m_renderer->BeginFrame();
            SDL_SetRenderDrawColor(m_renderer->Handle(), 0x2A, 0x0A, 0x0A, 0xFF);
            SDL_RenderClear(m_renderer->Handle());
            m_renderSystem->Render(m_registry, m_camera);
            m_renderer->EndFrame();
            continue;
        }

        // *** PAUSED STATE ***
        if (m_appState == AppState::Paused) {
            if (escNow) { m_appState = AppState::Playing; }
            SDL_SetWindowTitle(m_window->Handle(), "PAUSED — Press ESC to resume");
            m_renderer->BeginFrame();
            SDL_SetRenderDrawColor(m_renderer->Handle(), 0x15, 0x15, 0x15, 0xFF);
            SDL_RenderClear(m_renderer->Handle());
            m_renderSystem->Render(m_registry, m_camera);
            m_renderer->EndFrame();
            continue;
        }

        // *** PLAYING STATE ***
        float scrollSpeed = GetScrollSpeed();
        float spawnInterval = GetSpawnInterval();
        bool jumpHeld = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_SPACE]
                     || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_UP]
                     || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W];

        // Pause toggle
        if (escNow) { m_appState = AppState::Paused; continue; }

        // Fixed timestep: physics
        accumulator += deltaTime;
        m_gameOver = false;
        while (accumulator >= FIXED_DT) {
            physics.FixedUpdate(m_registry, FIXED_DT);
            collision.FixedUpdate(m_registry, m_eventBus, m_gameOver, m_score);
            accumulator -= FIXED_DT;
        }

        // Detect game over from collision
        if (m_gameOver) { m_appState = AppState::GameOver; continue; }

        // Jump
        if (jumpHeld) {
            auto view = m_registry.view<engine::physics::VelocityComponent, engine::physics::PlayerTag>();
            for (auto entity : view) {
                auto& vel = view.get<engine::physics::VelocityComponent>(entity);
                if (std::abs(vel.velocity.y) < 1.0f) { vel.velocity.y = JUMP_FORCE; }
            }
        }

        // Keep player at fixed X
        for (auto e : m_registry.view<engine::physics::PlayerTag>()) {
            auto& pos = m_registry.get<engine::renderer::TransformComponent>(e);
            pos.position.x = 100.0f;
        }

        // Spawn obstacles with difficulty scaling
        m_spawnTimer += deltaTime;
        if (m_spawnTimer >= spawnInterval) {
            m_spawnTimer -= spawnInterval;
            if (m_obstacleTexture) {
                auto obs = m_registry.create();
                m_registry.emplace<engine::renderer::TransformComponent>(obs,
                    core::Vec2f(1280.0f, 630.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
                m_registry.emplace<engine::renderer::SpriteComponent>(obs,
                    m_obstacleTexture, core::Rectf(0, 0, 40, 50), 2);
                m_registry.emplace<engine::physics::AABBComponent>(obs,
                    core::Rectf(0, 0, 40, 50));
            }
            SpawnCoins();
        }

        // Scroll all non-player, non-ground entities left
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
        SDL_SetWindowTitle(m_window->Handle(),
            std::format("Endless Runner — Score: {}  Speed: {:.0f}  Interval: {:.1f}s",
                m_score, scrollSpeed, spawnInterval).c_str());

        // Render
        m_renderer->BeginFrame();
        m_renderSystem->Render(m_registry, m_camera);
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
    m_renderSystem.reset();
    m_renderer.reset();
    m_stateMachine.reset();
    m_input.reset();
    m_window.reset();
    SDL_Quit();
    m_running = false;
}

} // namespace engine::application
