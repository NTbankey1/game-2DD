#include "Application.hpp"
#include "engine/platform/sdl3/SDLWindow.hpp"
#include "engine/platform/sdl3/SDLInputDevice.hpp"
#include "engine/scene/GameStateMachine.hpp"
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
        m_stateMachine->Render();
    }

    spdlog::info("Application::Run ended");
}

void Application::Shutdown() {
    if (!m_running && !m_window) return;
    spdlog::info("Application::Shutdown");
    m_stateMachine.reset();
    m_input.reset();
    m_window.reset();
    SDL_Quit();
    m_running = false;
}

} // namespace engine::application
