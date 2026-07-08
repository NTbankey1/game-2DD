#include "SDLRenderer.hpp"
#include "SDLWindow.hpp"
#include <spdlog/spdlog.h>

namespace engine::platform::sdl3 {

SDLRenderer::SDLRenderer(const SDLWindow& window) : m_window(window) {}
SDLRenderer::~SDLRenderer() { Shutdown(); }

bool SDLRenderer::Initialize() {
    spdlog::info("SDLRenderer::Initialize");
    m_renderer = SDL_CreateRenderer(m_window.Handle(), nullptr);
    if (!m_renderer) {
        spdlog::error("SDL_CreateRenderer failed: {}", SDL_GetError());
        return false;
    }
    if (!SDL_SetRenderVSync(m_renderer, 1)) {
        spdlog::warn("VSync not available: {}", SDL_GetError());
    }
    spdlog::info("SDLRenderer initialized");
    return true;
}

void SDLRenderer::Shutdown() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
        spdlog::info("SDLRenderer destroyed");
    }
}

void SDLRenderer::BeginFrame() {
    SDL_SetRenderDrawColor(m_renderer, 0x12, 0x12, 0x12, 0xFF);
    SDL_RenderClear(m_renderer);
}

void SDLRenderer::EndFrame() {
    SDL_RenderPresent(m_renderer);
}

} // namespace engine::platform::sdl3
