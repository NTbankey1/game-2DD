#include "PausedState.hpp"
#include "engine/application/Application.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace game {

static constexpr float W = 1280.0f;

PausedState::PausedState(engine::application::Application& app)
    : m_app(app) {}

PausedState::~PausedState() = default;

void PausedState::OnEnter() {
    spdlog::info("PausedState::OnEnter");
}

void PausedState::OnExit() {
    spdlog::info("PausedState::OnExit");
}

void PausedState::FixedUpdate(float /*dt*/) {}

void PausedState::Update(float /*dt*/) {
    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE]) {
        m_app.SetAppState(engine::application::AppState::Playing);
        m_app.States().PopScene();
    }
}

void PausedState::Render() {
    m_app.BeginFrame();
    m_app.DrawGradientBackground(0x10, 0x12, 0x20, 0x08, 0x0A, 0x18);
    m_app.DrawParallaxMountains();
    m_app.DrawSemiTransparentOverlay();
    m_app.DrawRoundedPanel(440, 280, 400, 140, 0x15, 0x15, 0x25, 220);
    m_app.Text().RenderString("- PAUSED -", 555, 300, 0xFF, 0xFF, 0xAA, 28);
    m_app.Text().RenderString("ESC to resume", 570, 350, 0xAA, 0xAA, 0x88, 16);
    m_app.EndFrame();
}

} // namespace game
