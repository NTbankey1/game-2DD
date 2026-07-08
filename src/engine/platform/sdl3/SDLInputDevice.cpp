#include "SDLInputDevice.hpp"

namespace engine::platform::sdl3 {

SDLInputDevice::SDLInputDevice() { m_keys.reserve(256); }

void SDLInputDevice::PollEvents() {
    m_quitRequested = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_quitRequested = true;
            break;
        case SDL_EVENT_KEY_DOWN:
            m_keys[event.key.scancode] = true;
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                m_quitRequested = true;
            }
            break;
        case SDL_EVENT_KEY_UP:
            m_keys[event.key.scancode] = false;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            m_mouseX = event.motion.x;
            m_mouseY = event.motion.y;
            break;
        }
    }
}

bool SDLInputDevice::IsKeyPressed(int key) {
    auto it = m_keys.find(static_cast<SDL_Scancode>(key));
    return it != m_keys.end() && it->second;
}

void SDLInputDevice::GetMousePos(float& x, float& y) {
    x = m_mouseX;
    y = m_mouseY;
}

} // namespace engine::platform::sdl3
