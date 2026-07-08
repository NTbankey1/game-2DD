#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace engine::platform::sdl3 {

/// SDL3 window wrapper — concrete ownership of SDL_Window*.
class SDLWindow {
public:
    static SDLWindow* Create(const std::string& title, int width, int height);
    ~SDLWindow();

    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;
    SDLWindow(SDLWindow&&) = delete;
    SDLWindow& operator=(SDLWindow&&) = delete;

    [[nodiscard]] SDL_Window* Handle() const noexcept { return m_window; }
    [[nodiscard]] int Width() const noexcept { return m_width; }
    [[nodiscard]] int Height() const noexcept { return m_height; }

private:
    SDLWindow(SDL_Window* window, int width, int height) noexcept;
    SDL_Window* m_window = nullptr;
    int m_width = 1280;
    int m_height = 720;
};

} // namespace engine::platform::sdl3
