#include "Application.hpp"
#include "engine/platform/sdl3/SDLWindow.hpp"
#include "engine/platform/sdl3/SDLInputDevice.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/renderer/RenderSystem.hpp"
#include "engine/physics/PhysicsSystem.hpp"
#include "engine/physics/CollisionSystem.hpp"
#include "engine/input/KeyEvent.hpp"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <spdlog/spdlog.h>

namespace engine::application {

Application::Application()
    : m_stateMachine(std::make_unique<engine::scene::GameStateMachine>()) {}

Application::~Application() { Shutdown(); }

static SDL_Texture* CreateSolidTexture(SDL_Renderer* renderer, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    SDL_Surface* surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) return nullptr;
    SDL_FillSurfaceRect(surface, nullptr,
        SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, r, g, b, a));
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return tex;
}

bool Application::Initialize() {
    spdlog::info("Application::Initialize");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    spdlog::info("SDL3 initialized");

    m_window.reset(engine::platform::sdl3::SDLWindow::Create("Endless Runner", 1280, 720));
    if (!m_window) return false;

    m_input = std::make_unique<engine::platform::sdl3::SDLInputDevice>();

    m_renderer = std::make_unique<engine::platform::sdl3::SDLRenderer>(*m_window);
    if (!m_renderer->Initialize()) return false;

    m_renderSystem = std::make_unique<engine::renderer::RenderSystem>(*m_renderer);
    m_camera.SetViewport(core::Vec2f(1280.0f, 720.0f));

    // ---- Player: red square 50x50 at top-left, falls with gravity ----
    m_playerTexture = CreateSolidTexture(m_renderer->Handle(), 50, 50, 0xFF, 0x33, 0x33, 0xFF);
    if (m_playerTexture) {
        auto player = m_registry.create();
        m_registry.emplace<engine::renderer::TransformComponent>(player,
            core::Vec2f(100.0f, 0.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
        m_registry.emplace<engine::renderer::SpriteComponent>(player,
            m_playerTexture, core::Rectf(0, 0, 50, 50), 1);
        m_registry.emplace<engine::physics::VelocityComponent>(player);
        m_registry.emplace<engine::physics::GravityComponent>(player, 980.0f);
        m_registry.emplace<engine::physics::AABBComponent>(player, core::Rectf(0, 0, 50, 50));
        m_registry.emplace<engine::physics::PlayerTag>(player);
        spdlog::info("Player entity created");
    }

    // ---- Ground: green bar at bottom ----
    m_groundTexture = CreateSolidTexture(m_renderer->Handle(), 1280, 40, 0x33, 0xCC, 0x33, 0xFF);
    if (m_groundTexture) {
        auto ground = m_registry.create();
        m_registry.emplace<engine::renderer::TransformComponent>(ground,
            core::Vec2f(0.0f, 680.0f), 0.0f, core::Vec2f(1.0f, 1.0f));
        m_registry.emplace<engine::renderer::SpriteComponent>(ground,
            m_groundTexture, core::Rectf(0, 0, 1280, 40), 0);
        m_registry.emplace<engine::physics::AABBComponent>(ground, core::Rectf(0, 0, 1280, 40));
        m_registry.emplace<engine::physics::GroundTag>(ground);
        spdlog::info("Ground entity created");
    }

    m_running = true;
    return true;
}

void Application::Run() {
    if (!m_running) {
        spdlog::warn("Application::Run called but not initialized");
        return;
    }
    spdlog::info("Application::Run started");

    engine::physics::PhysicsSystem physics;
    engine::physics::CollisionSystem collision;

    float accumulator = 0.0f;
    std::uint64_t prevCounter = SDL_GetPerformanceCounter();
    const std::uint64_t counterFreq = SDL_GetPerformanceFrequency();

    while (m_running) {
        m_input->PollEvents();
        if (m_input->QuitRequested()) {
            m_running = false;
            break;
        }

        // Publish KeyEvent for each pressed key
        {
            static constexpr SDL_Scancode tracked[] = {
                SDL_SCANCODE_SPACE, SDL_SCANCODE_UP, SDL_SCANCODE_W,
                SDL_SCANCODE_DOWN, SDL_SCANCODE_S,
                SDL_SCANCODE_ESCAPE, SDL_SCANCODE_RETURN
            };
            for (auto sc : tracked) {
                bool pressed = SDL_GetKeyboardState(nullptr)[sc];
                m_eventBus.Publish(engine::input::KeyEvent(
                    static_cast<int>(sc), pressed
                ));
            }
        }

        std::uint64_t currCounter = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(currCounter - prevCounter) / static_cast<float>(counterFreq);
        prevCounter = currCounter;
        if (deltaTime > 0.25f) deltaTime = 0.25f;
        m_frameTime = deltaTime;

        // Fixed timestep: physics + collision
        accumulator += deltaTime;
        while (accumulator >= FIXED_DT) {
            if (!m_stateMachine->Empty()) m_stateMachine->FixedUpdate(FIXED_DT);
            physics.FixedUpdate(m_registry, FIXED_DT);
            collision.FixedUpdate(m_registry, m_eventBus);
            accumulator -= FIXED_DT;
        }

        if (!m_stateMachine->Empty()) m_stateMachine->Update(deltaTime);

        // Render
        m_renderer->BeginFrame();
        m_stateMachine->Render();
        m_renderSystem->Render(m_registry, m_camera);
        m_renderer->EndFrame();
    }

    spdlog::info("Application::Run ended");
}

void Application::Shutdown() {
    if (!m_running && !m_window) return;
    spdlog::info("Application::Shutdown");

    if (m_playerTexture) { SDL_DestroyTexture(m_playerTexture); m_playerTexture = nullptr; }
    if (m_groundTexture) { SDL_DestroyTexture(m_groundTexture); m_groundTexture = nullptr; }
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
