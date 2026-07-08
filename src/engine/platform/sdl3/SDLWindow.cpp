#include "SDLWindow.hpp"
#include <spdlog/spdlog.h>

namespace engine::platform::sdl3 {

SDLWindow* SDLWindow::Create(const std::string& title, int width, int height) {
    SDL_Window* window = SDL_CreateWindow(
        title.c_str(),
        width, height,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
        return nullptr;
    }
    spdlog::info("SDLWindow created: {} ({}x{})", title, width, height);
    return new SDLWindow(window, width, height);
}

SDLWindow::SDLWindow(SDL_Window* window, int width, int height) noexcept
    : m_window(window), m_width(width), m_height(height) {}

SDLWindow::~SDLWindow() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        spdlog::info("SDLWindow destroyed");
    }
}

} // namespace engine::platform::sdl3
