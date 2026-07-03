#pragma once

#include "engine/application/IApplication.hpp"
#include <memory>

namespace engine::platform::sdl3 {
    class SDLWindow;
    class SDLInputDevice;
}
namespace engine::scene { class GameStateMachine; }

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
    const engine::scene::GameStateMachine& States() const { return *m_stateMachine; }

private:
    std::unique_ptr<engine::platform::sdl3::SDLWindow> m_window;
    std::unique_ptr<engine::platform::sdl3::SDLInputDevice> m_input;
    std::unique_ptr<engine::scene::GameStateMachine> m_stateMachine;

    float m_frameTime = 0.0f;
    static constexpr float FIXED_DT = 1.0f / 60.0f;
    bool m_running = false;
};

} // namespace engine::application
