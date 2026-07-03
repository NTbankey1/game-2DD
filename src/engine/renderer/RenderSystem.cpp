#include "RenderSystem.hpp"
#include "engine/platform/sdl3/SDLRenderer.hpp"
#include "engine/camera/Camera.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>

namespace engine::renderer {

RenderSystem::RenderSystem(engine::platform::sdl3::SDLRenderer& renderer)
    : m_renderer(renderer) {}

void RenderSystem::Render(entt::registry& registry, engine::Camera& camera) {
    std::vector<entt::entity> drawables;
    for (auto [entity, sprite, transform] : registry.view<SpriteComponent, TransformComponent>().each()) {
        drawables.push_back(entity);
    }

    std::sort(drawables.begin(), drawables.end(), [&](entt::entity a, entt::entity b) {
        return registry.get<SpriteComponent>(a).renderLayer < registry.get<SpriteComponent>(b).renderLayer;
    });

    for (auto entity : drawables) {
        const auto& sprite = registry.get<SpriteComponent>(entity);
        const auto& transform = registry.get<TransformComponent>(entity);
        if (!sprite.texture) continue;

        core::Vec2f screenPos = transform.position - camera.GetPosition();

        SDL_FRect dest{
            screenPos.x, screenPos.y,
            sprite.sourceRect.size.x * transform.scale.x,
            sprite.sourceRect.size.y * transform.scale.y
        };

        const SDL_FRect* src = (sprite.sourceRect.size.x > 0 && sprite.sourceRect.size.y > 0)
            ? reinterpret_cast<const SDL_FRect*>(&sprite.sourceRect)
            : nullptr;

        SDL_RenderTexture(m_renderer.Handle(), sprite.texture, src, &dest);
    }
}

} // namespace engine::renderer
