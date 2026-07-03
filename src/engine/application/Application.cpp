#include "Application.hpp"
#include "engine/platform/sdl3/SDLWindow.hpp"
#include "engine/platform/sdl3/SDLInputDevice.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/renderer/RenderSystem.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::application {

Application::Application()
    : m_stateMachine(std::make_unique<engine::scene::GameStateMachine>()) {}

Application::~Application() { Shutdown(); }

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

    // Create test texture: 50x50 red square
    SDL_Surface* surface = SDL_CreateSurface(50, 50, SDL_PIXELFORMAT_RGBA8888);
    if (surface) {
        SDL_FillSurfaceRect(surface, nullptr, SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888), nullptr, 0xFF, 0x33, 0x33, 0xFF));
        m_testTexture = SDL_CreateTextureFromSurface(m_renderer->Handle(), surface);
        SDL_DestroySurface(surface);

        // Create a test entity
        auto entity = m_registry.create();
        m_registry.emplace<engine::renderer::TransformComponent>(entity,
            core::Vec2f(100.0f, 300.0f), 0.0f, core::Vec2f(2.0f, 2.0f));
        m_registry.emplace<engine::renderer::SpriteComponent>(entity,
            m_testTexture, core::Rectf(0, 0, 50, 50), 0);
        spdlog::info("Test entity created with red square texture");
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

    float accumulator = 0.0f;
    std::uint64_t prevCounter = SDL_GetPerformanceCounter();
    const std::uint64_t counterFreq = SDL_GetPerformanceFrequency();

    while (m_running) {
        m_input->PollEvents();
        if (m_input->QuitRequested()) {
            m_running = false;
            break;
        }

        std::uint64_t currCounter = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(currCounter - prevCounter) / static_cast<float>(counterFreq);
        prevCounter = currCounter;
        if (deltaTime > 0.25f) deltaTime = 0.25f;
        m_frameTime = deltaTime;

        accumulator += deltaTime;
        while (accumulator >= FIXED_DT) {
            if (!m_stateMachine->Empty()) m_stateMachine->FixedUpdate(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        if (!m_stateMachine->Empty()) m_stateMachine->Update(deltaTime);

        // Render frame
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

    if (m_testTexture) {
        SDL_DestroyTexture(m_testTexture);
        m_testTexture = nullptr;
    }
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
