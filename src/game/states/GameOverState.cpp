#include "GameOverState.hpp"
#include "ExplorationState.hpp"
#include "engine/application/Application.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <string>
#include <cmath>

namespace game {

GameOverState::GameOverState(engine::application::Application& app)
    : m_app(app) {}

GameOverState::~GameOverState() = default;

void GameOverState::OnEnter() {
    spdlog::info("GameOverState::OnEnter");
    m_app.UpdateHighScore();
    m_debounceEnter = true;
}

void GameOverState::OnExit() {
    spdlog::info("GameOverState::OnExit");
}

void GameOverState::FixedUpdate(float /*dt*/) {}

void GameOverState::Update(float /*dt*/) {
    bool confirmNow = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_RETURN];
    if (confirmNow && m_debounceEnter) { confirmNow = false; }
    else if (!confirmNow) { m_debounceEnter = false; }

    if (confirmNow) {
        m_app.ResetGame();
        m_app.SetAppState(engine::application::AppState::Menu);
        m_app.SetScore(0);
        m_app.SetDebounceEnter(true);
        m_app.Events().Clear();
        while (!m_app.States().Empty())
            m_app.States().PopScene();
    }
}

void GameOverState::Render() {
    m_app.BeginFrame();
    m_app.DrawGradientBackground(0x12, 0x08, 0x08, 0x08, 0x04, 0x04);
    m_app.DrawParallaxMountains();
    m_app.DrawSemiTransparentOverlay();

    m_app.DrawRoundedPanel(340, 160, 600, 360, 0x10, 0x08, 0x08, 220);

    m_app.Text().RenderString("GAME OVER", 460, 175, 0xFF, 0x55, 0x55, 36);
    m_app.Text().RenderString("GAME OVER", 461, 176, 0x88, 0x22, 0x22, 36); // shadow

    m_app.Text().RenderString("───", 610, 215, 0x88, 0x44, 0x44, 14);

    m_app.Text().RenderString("FINAL SCORE", 500, 245, 0xFF, 0xCC, 0xCC, 22);
    std::string scoreStr = std::to_string(m_app.Score());
    float sw = static_cast<float>(m_app.Text().TextWidth(scoreStr.c_str(), 30));
    m_app.Text().RenderString(scoreStr.c_str(), 640 - sw / 2.0f, 268, 0xFF, 0xFF, 0xFF, 30);

    // Stats line
    int distM = static_cast<int>(m_app.Distance() / 10.0f);
    m_app.Text().RenderString(("Dist: " + std::to_string(distM) + "m").c_str(), 440, 315, 0x88, 0xCC, 0x88, 16);
    m_app.Text().RenderString(("Coins: " + std::to_string(m_app.Coins())).c_str(), 580, 315, 0xFF, 0xDD, 0x00, 16);
    m_app.Text().RenderString(("Best combo: x" + std::to_string(m_app.MaxCombo())).c_str(), 700, 315, 0xAA, 0x88, 0x44, 14);

    std::string bestStr = "Best: " + std::to_string(m_app.HighScore());
    float bw = static_cast<float>(m_app.Text().TextWidth(bestStr.c_str(), 16));
    m_app.Text().RenderString(bestStr.c_str(), 640 - bw / 2.0f, 350, 0xCC, 0x88, 0x44, 16);

    float gp = 0.7f + 0.3f * std::sin(m_app.MenuTimer() * 3.0f);
    uint8_t gpr = static_cast<uint8_t>(85.0f + 85.0f * gp);
    m_app.Text().RenderString("Press ENTER to restart", 510, 395, gpr, gpr, static_cast<uint8_t>(187.0f + 68.0f * gp), 18);

    m_app.EndFrame();
}

} // namespace game
