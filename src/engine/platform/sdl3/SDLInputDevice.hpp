#pragma once

#include "engine/input/IInputDevice.hpp"
#include <SDL3/SDL.h>
#include <unordered_map>

namespace engine::platform::sdl3 {

class SDLInputDevice : public engine::IInputDevice {
public:
    explicit SDLInputDevice();
    ~SDLInputDevice() override = default;

    void PollEvents() override;
    bool IsKeyPressed(int key) override;
    void GetMousePos(float& x, float& y) override;

    [[nodiscard]] bool QuitRequested() const noexcept { return m_quitRequested; }

private:
    bool m_quitRequested = false;
    std::unordered_map<SDL_Scancode, bool> m_keys;
    float m_mouseX = 0, m_mouseY = 0;
};

} // namespace engine::platform::sdl3
