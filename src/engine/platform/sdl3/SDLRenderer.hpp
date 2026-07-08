#pragma once

#include "engine/renderer/IRenderer.hpp"
#include <SDL3/SDL.h>

namespace engine::platform::sdl3 {

class SDLWindow;

class SDLRenderer : public engine::IRenderer {
public:
    explicit SDLRenderer(const SDLWindow& window);
    ~SDLRenderer() override;

    bool Initialize() override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;

    [[nodiscard]] SDL_Renderer* Handle() const noexcept { return m_renderer; }

private:
    const SDLWindow& m_window;
    SDL_Renderer* m_renderer = nullptr;
};

} // namespace engine::platform::sdl3
