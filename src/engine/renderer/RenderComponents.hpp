#pragma once

#include "core/core.hpp"
#include <SDL3/SDL.h>

namespace engine::renderer {

struct TransformComponent {
    core::Vec2f position{};
    float rotation = 0.0f;
    core::Vec2f scale{1.0f, 1.0f};
};

struct SpriteComponent {
    SDL_Texture* texture = nullptr;
    core::Rectf sourceRect{};
    int renderLayer = 0;
};

} // namespace engine::renderer
