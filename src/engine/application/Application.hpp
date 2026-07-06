#pragma once

#include "engine/application/IApplication.hpp"
#include "engine/camera/Camera.hpp"
#include "core/events/EventBus.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include <entt/entt.hpp>
#include <memory>
#include <cstdint>

struct SDL_Texture;

namespace engine::platform::sdl3 {
    class SDLWindow;
    class SDLInputDevice;
    class SDLRenderer;
}
namespace engine::scene { class GameStateMachine; }
namespace engine::renderer { class RenderSystem; }
namespace engine::physics { class PhysicsSystem; class CollisionSystem; }

namespace engine::application {

/// Tag for collectable coins
struct CoinTag {};

enum class AppState { Menu, Playing, Paused, GameOver };

class Application : public engine::IApplication {
public:
    Application();
    ~Application() override;

    bool Initialize() override;
    void Run() override;
    void Shutdown() override;
    float GetFrameTime() const override { return m_frameTime; }

    // Subsystem access
    engine::scene::GameStateMachine& States() { return *m_stateMachine; }
    entt::registry& Registry() { return m_registry; }
    engine::Camera& Camera() { return m_camera; }
    core::events::EventBus& Events() { return m_eventBus; }
    engine::platform::sdl3::SDLRenderer& Renderer() { return *m_renderer; }
    engine::renderer::TextRenderer& Text() { return *m_text; }
    engine::renderer::RenderSystem& RenderSys() { return *m_renderSystem; }
    engine::platform::sdl3::SDLInputDevice& InputDev() { return *m_input; }

    // Game data access
    int Score() const { return m_score; }
    int HighScore() const { return m_highScore; }
    int Coins() const { return m_coins; }
    int Combo() const { return m_combo; }
    int MaxCombo() const { return m_maxCombo; }
    int Phase() const { return m_phase; }
    float Distance() const { return m_distance; }
    float FrameTime() const { return m_frameTime; }
    float MenuTimer() const { return m_menuTimer; }
    float ScrollSpeed() const { return GetScrollSpeed(); }
    bool IsGameOver() const { return m_gameOver; }
    AppState GetAppState() const { return m_appState; }

    void SetAppState(AppState s) { m_appState = s; }
    void SetGameOver(bool v) { m_gameOver = v; }
    void AddScore(int s) { m_score += s; }
    void SetScore(int s) { m_score = s; }
    void UpdateHighScore() { if (m_score > m_highScore) m_highScore = m_score; }
    void AddCoin() { m_coins++; }
    void SetCombo(int c) { m_combo = c; }
    void UpdateMaxCombo() { if (m_combo > m_maxCombo) m_maxCombo = m_combo; }
    void SetDistance(float d) { m_distance = d; }
    void SetPhase(int p) { m_phase = p; }
    void SetDebounceEnter(bool v) { m_debounceEnter = v; }

    // Frame lifecycle (states call these)
    void BeginFrame() { m_renderer->BeginFrame(); }
    void EndFrame() { m_renderer->EndFrame(); }

    // Drawing helpers
    void DrawGradientBackground(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2);
    void DrawStars(float brightnessScale);
    void DrawClouds();
    void DrawParallaxMountains();
    void DrawSemiTransparentOverlay();
    void DrawRoundedPanel(float x, float y, float w, float h, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // Particles
    void EmitParticles(float x, float y, int count, uint8_t r, uint8_t g, uint8_t b, float spread);
    void UpdateParticles(float dt);
    void RenderParticles();

    // Score popups
    void SpawnScorePopup(int score, float x, float y);
    void UpdatePopups(float dt);
    void RenderPopups();

    // Spawning
    void SpawnPlayer();
    void SpawnGround();
    void SpawnCoin();
    void ResetGame();
    float GetScrollSpeed() const;
    float GetSpawnInterval() const;

    // Animation state refs
    struct PlayerAnimState {
        float jumpSquashTimer = 0.0f;
        bool wasGrounded = true;
        bool wasAlive = true;
    };
    PlayerAnimState& Anim() { return m_playerAnim; }
    int& PlayerFrame() { return m_playerFrame; }
    float& AnimTimer() { return m_animTimer; }
    float& ShakeAmount() { return m_shakeAmount; }
    float& SpawnTimer() { return m_spawnTimer; }

    // Trail
    struct TrailPoint { float x{}, y{}; float life{}; };
    static constexpr int MAX_TRAIL = 10;
    int& TrailHead() { return m_trailHead; }
    TrailPoint* TrailPoints() { return m_trail; }

    // Parallax state
    float& BgOffset1() { return m_bgOffset1; }
    float& BgOffset2() { return m_bgOffset2; }
    float& CloudOffset() { return m_cloudOffset; }

    // Cached textures
    SDL_Texture* PlayerSheetTexturePtr() const { return m_playerSheetTexture; }
    SDL_Texture* GameOverTextPtr() const { return m_gameOverText; }
    SDL_Texture* PausedTextPtr() const { return m_pausedText; }
    SDL_Texture* TitleTextPtr() const { return m_titleText; }
    SDL_Texture* PromptTextPtr() const { return m_promptText; }
    SDL_Texture* CoinTexturePtr() const { return m_coinTexture; }
    SDL_Texture* ObstacleTexturePtr() const { return m_obstacleTexture; }

private:
    struct Particle {
        float x{}, y{};
        float vx{}, vy{};
        float life{}, maxLife{};
        uint8_t r{}, g{}, b{}, a{}, size{};
    };
    static constexpr int MAX_PARTICLES = 80;
    Particle m_particles[MAX_PARTICLES];
    int m_particleCount = 0;

    PlayerAnimState m_playerAnim;
    int m_playerFrame = 0;
    float m_animTimer = 0.0f;
    float m_shakeAmount = 0.0f;

    TrailPoint m_trail[MAX_TRAIL];
    int m_trailHead = 0;

    struct ScorePopup {
        float x{}, y{};
        float life{};
        int score{};
        bool active = false;
    };
    static constexpr int MAX_POPUPS = 8;
    ScorePopup m_popups[MAX_POPUPS];

    struct Star { float x, y, phase; uint8_t baseBrightness; };
    static constexpr int STAR_COUNT = 100;
    Star m_stars[STAR_COUNT];

    std::unique_ptr<engine::platform::sdl3::SDLWindow> m_window;
    std::unique_ptr<engine::platform::sdl3::SDLInputDevice> m_input;
    std::unique_ptr<engine::platform::sdl3::SDLRenderer> m_renderer;
    std::unique_ptr<engine::scene::GameStateMachine> m_stateMachine;
    std::unique_ptr<engine::renderer::RenderSystem> m_renderSystem;
    std::unique_ptr<engine::renderer::TextRenderer> m_text;

    engine::Camera m_camera;
    core::events::EventBus m_eventBus;
    entt::registry m_registry;
    SDL_Texture* m_playerSheetTexture = nullptr;
    SDL_Texture* m_groundTexture = nullptr;
    SDL_Texture* m_obstacleTexture = nullptr;
    SDL_Texture* m_coinTexture = nullptr;
    SDL_Texture* m_titleText = nullptr;
    SDL_Texture* m_promptText = nullptr;
    SDL_Texture* m_gameOverText = nullptr;
    SDL_Texture* m_pausedText = nullptr;

    AppState m_appState = AppState::Menu;
    bool m_gameOver = false;
    int m_score = 0;
    int m_highScore = 0;
    int m_coins = 0;
    int m_combo = 0;
    int m_maxCombo = 0;
    float m_distance = 0.0f;
    int m_phase = 1;
    float m_spawnTimer = 0.0f;
    bool m_debounceEnter = false;

    float m_bgOffset1 = 0.0f;
    float m_bgOffset2 = 0.0f;
    float m_cloudOffset = 0.0f;
    float m_menuTimer = 0.0f;
    float m_frameTime = 0.0f;
    static constexpr float FIXED_DT = 1.0f / 60.0f;
    bool m_running = false;

    // Internal helpers
    void GenerateStars();
    void CacheStaticTexts();
};

} // namespace engine::application
