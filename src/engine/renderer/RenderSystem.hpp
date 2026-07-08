#pragma once

#include <entt/entt.hpp>
#include "engine/renderer/RenderComponents.hpp"

namespace engine::platform::sdl3 { class SDLRenderer; }
namespace engine { class Camera; }

namespace engine::renderer {

class RenderSystem {
public:
    explicit RenderSystem(engine::platform::sdl3::SDLRenderer& renderer);

    void Render(entt::registry& registry, engine::Camera& camera);

private:
    engine::platform::sdl3::SDLRenderer& m_renderer;
};

} // namespace engine::renderer
