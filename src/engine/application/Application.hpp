#pragma once

#include "engine/application/IApplication.hpp"
#include "engine/camera/Camera.hpp"
#include <entt/entt.hpp>
#include <memory>

struct SDL_Texture;

namespace engine::platform::sdl3 {
    class SDLWindow;
    class SDLInputDevice;
    class SDLRenderer;
}
namespace engine::scene { class GameStateMachine; }
namespace engine::renderer { class RenderSystem; }

namespace engine::application {

class Application : public engine::IApplication {
public:
    Application();
    ~Application() override;

    bool Initialize() override;
    void Run() override;
    void Shutdown() override;
    float GetFrameTime() const override { return m_frameTime; }

    engine::scene::GameStateMachine& States() { return *m_stateMachine; }
    entt::registry& Registry() { return m_registry; }
    engine::Camera& Camera() { return m_camera; }

private:
    std::unique_ptr<engine::platform::sdl3::SDLWindow> m_window;
    std::unique_ptr<engine::platform::sdl3::SDLInputDevice> m_input;
    std::unique_ptr<engine::platform::sdl3::SDLRenderer> m_renderer;
    std::unique_ptr<engine::scene::GameStateMachine> m_stateMachine;
    std::unique_ptr<engine::renderer::RenderSystem> m_renderSystem;

    engine::Camera m_camera;
    entt::registry m_registry;
    SDL_Texture* m_testTexture = nullptr;

    float m_frameTime = 0.0f;
    static constexpr float FIXED_DT = 1.0f / 60.0f;
    bool m_running = false;
};

} // namespace engine::application
