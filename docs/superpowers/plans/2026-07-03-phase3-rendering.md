# Phase 3: SDL3 Rendering Backend & Camera Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use inline execution for this plan.

**Goal:** Initialize SDL_Renderer, draw a colored rectangle on screen (smoke test), implement a 2D camera with parallax-ready transform, render through the Application loop.

**Architecture:** Concrete `SDLRenderer` class implementing `engine::IRenderer`. `Camera` component manages view offset and parallax layers. `RenderSystem` iterates ECS entities with `SpriteComponent` + `TransformComponent` and draws them. MenuState drives the render pipeline.

**Tech Stack:** SDL3 (Renderer API), EnTT (ECS components), all existing Phase 1-2 code.

## Global Constraints

- SDL_Renderer created from SDL_Window (already owned by Application)
- All rendering goes through `BeginFrame()` / `EndFrame()` lifecycle
- Camera is a standalone class, not tied to SDL3 — testable without a window
- No SDL3 headers leak into `game/` or `core/`
- Must not crash when window is minimized or zero-size
- Procedural texture (colored rect) for Phase 3 — no asset loading yet

---
### Task 1: SDLRenderer — Concrete Renderer Implementation

**Files:**
- Create: `src/engine/platform/sdl3/SDLRenderer.hpp`
- Create: `src/engine/platform/sdl3/SDLRenderer.cpp`

**Interfaces:**
- Consumes: `engine::IRenderer` (abstract), `SDLWindow` (for window handle)
- Produces: Concrete `SDLRenderer` class

```cpp
// SDLRenderer.hpp
#pragma once

#include "engine/renderer/IRenderer.hpp"
#include <SDL3/SDL.h>

namespace engine::platform::sdl3 {

class SDLWindow;  // forward decl

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
```

```cpp
// SDLRenderer.cpp
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
    spdlog::info("SDLRenderer initialized (VSync={})", SDL_GetRendererVSync(m_renderer));
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
    SDL_SetRenderDrawColor(m_renderer, 0x12, 0x12, 0x12, 0xFF);  // dark grey
    SDL_RenderClear(m_renderer);
}

void SDLRenderer::EndFrame() {
    SDL_RenderPresent(m_renderer);
}

} // namespace engine::platform::sdl3
```

---
### Task 2: Camera Component — 2D View Offset & Parallax

**Files:**
- Create: `src/engine/camera/Camera.hpp`
- Create: `tests/unit/engine/test_camera.cpp`

**Interfaces:**
- Consumes: Pure math types (Vec2f, Rectf)
- Produces: `engine::Camera` class

```cpp
// Camera.hpp
#pragma once

#include "core/core.hpp"
#include <cstdint>

namespace engine {

/// 2D Camera with parallax layer support.
/// Manages view offset and zoom for the renderer.
class Camera {
public:
    Camera() = default;

    /// Set absolute position (bottom-left of view)
    void SetPosition(core::Vec2f pos) noexcept { m_position = pos; }
    [[nodiscard]] core::Vec2f GetPosition() const noexcept { return m_position; }

    /// Move camera by delta
    void Move(core::Vec2f delta) noexcept { m_position += delta; }

    /// Set zoom level (1.0 = normal)
    void SetZoom(float zoom) noexcept { m_zoom = zoom; }
    [[nodiscard]] float GetZoom() const noexcept { return m_zoom; }

    /// Viewport size in pixels
    void SetViewport(core::Vec2f size) noexcept { m_viewport = size; }
    [[nodiscard]] core::Vec2f GetViewport() const noexcept { return m_viewport; }

    /// Get camera bounds in world space
    [[nodiscard]] core::Rectf GetBounds() const noexcept {
        return core::Rectf(m_position, m_viewport * m_zoom);
    }

    /// Get the render offset for a given parallax factor.
    /// factor=1.0 → follows camera exactly (foreground)
    /// factor=0.3 → moves slowly (far background)
    [[nodiscard]] core::Vec2f GetParallaxOffset(float factor) const noexcept {
        return m_position * factor;
    }

private:
    core::Vec2f m_position{};
    core::Vec2f m_viewport{1280.0f, 720.0f};
    float m_zoom = 1.0f;
};

} // namespace engine
```

```cpp
// tests/unit/engine/test_camera.cpp
#include <catch2/catch_test_macros.hpp>
#include "engine/camera/Camera.hpp"

using namespace core;
using namespace engine;

TEST_CASE("Camera: default state", "[engine][camera]") {
    Camera cam;
    CHECK(cam.GetPosition() == Vec2f(0, 0));
    CHECK(cam.GetZoom() == 1.0f);
    CHECK(cam.GetViewport() == Vec2f(1280, 720));
}

TEST_CASE("Camera: set position", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(100, 200));
    CHECK(cam.GetPosition() == Vec2f(100, 200));
}

TEST_CASE("Camera: move", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(100, 100));
    cam.Move(Vec2f(10, -20));
    CHECK(cam.GetPosition() == Vec2f(110, 80));
}

TEST_CASE("Camera: zoom", "[engine][camera]") {
    Camera cam;
    cam.SetZoom(2.0f);
    CHECK(cam.GetZoom() == 2.0f);
}

TEST_CASE("Camera: bounds", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(100, 50));
    cam.SetViewport(Vec2f(800, 600));
    auto b = cam.GetBounds();
    CHECK(b.position == Vec2f(100, 50));
    CHECK(b.size == Vec2f(800, 600));
}

TEST_CASE("Camera: parallax offset", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(500, 300));
    auto fg = cam.GetParallaxOffset(1.0f);
    CHECK(fg == Vec2f(500, 300));
    auto bg = cam.GetParallaxOffset(0.25f);
    CHECK(bg == Vec2f(125, 75));
}
```

---
### Task 3: ECS Render Components (Transform, Sprite)

**Files:**
- Create: `src/engine/renderer/RenderComponents.hpp`

**Interfaces:**
- Consumes: EnTT (via core::ecs), Vec2f, Rectf
- Produces: `TransformComponent`, `SpriteComponent`

```cpp
// RenderComponents.hpp
#pragma once

#include "core/core.hpp"
#include <SDL3/SDL.h>

namespace engine::renderer {

/// Position, rotation, scale for any renderable entity
struct TransformComponent {
    core::Vec2f position{};
    float rotation = 0.0f;  // degrees
    core::Vec2f scale{1.0f, 1.0f};
};

/// Reference to an SDL_Texture with source rect
struct SpriteComponent {
    SDL_Texture* texture = nullptr;
    core::Rectf sourceRect{};   // portion of the texture to use
    int renderLayer = 0;        // draw order (lower = drawn first)
};

} // namespace engine::renderer
```

---
### Task 4: RenderSystem — ECS Render Pipeline

**Files:**
- Create: `src/engine/renderer/RenderSystem.hpp`
- Create: `src/engine/renderer/RenderSystem.cpp`

**Interfaces:**
- Consumes: `SDLRenderer`, `Camera`, ECS Registry with TransformComponent + SpriteComponent
- Produces: `RenderSystem` class

```cpp
// RenderSystem.hpp
#pragma once

#include <entt/entt.hpp>

namespace engine::renderer {
    class RenderSystem;
}

#include "engine/renderer/RenderComponents.hpp"

namespace engine::platform::sdl3 { class SDLRenderer; }
namespace engine { class Camera; }

namespace engine::renderer {

class RenderSystem {
public:
    explicit RenderSystem(engine::platform::sdl3::SDLRenderer& renderer);

    /// Draw all entities with SpriteComponent + TransformComponent
    void Render(entt::registry& registry, engine::Camera& camera);

private:
    engine::platform::sdl3::SDLRenderer& m_renderer;
};

} // namespace engine::renderer
```

```cpp
// RenderSystem.cpp
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
    // Collect all drawable entities sorted by layer
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

        // World position → screen position via camera
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
```

---
### Task 5: Wire Renderer into Application + Test Scene

**Files:**
- Modify: `src/engine/application/Application.hpp`
- Modify: `src/engine/application/Application.cpp`
- Modify: `src/game/engine_impl/MenuState.hpp`
- Modify: `src/game/engine_impl/MenuState.cpp`

**Changes:**

In `Application.hpp`, add:
```cpp
#include "engine/renderer/IRenderer.hpp"  // for access
#include "engine/camera/Camera.hpp"
#include "engine/renderer/RenderSystem.hpp"
// members:
std::unique_ptr<engine::platform::sdl3::SDLRenderer> m_renderer;
engine::Camera m_camera;
engine::renderer::RenderSystem m_renderSystem;
```

In `Application.cpp`, in `Initialize()`:
```cpp
// After window creation:
m_renderer = std::make_unique<engine::platform::sdl3::SDLRenderer>(*m_window);
if (!m_renderer->Initialize()) return false;

m_camera.SetViewport(core::Vec2f(1280.0f, 720.0f));
```

In `Application::Run()`, replace `m_stateMachine->Render()` with:
```cpp
m_renderer->BeginFrame();
m_stateMachine->Render();
m_renderSystem.Render(registry, m_camera);  // if MenuState owns a registry
m_renderer->EndFrame();
```

MenuState owns an ECS Registry, creates a test entity with Transform + Sprite.

---
### Task 6: Build & Integration Test

**Files:** None (verification only)

- [ ] **Step 1: Clean build**

```bash
cmake --build build/debug --parallel $(nproc) 2>&1 | grep -E "error:|FAILED|Linking" | head -10
```

- [ ] **Step 2: Run unit tests**

```bash
./build/debug/tests/unit_tests --success 2>&1 | tail -5
```

Expected: `All tests passed` including camera tests.

- [ ] **Step 3: Runtime smoke test**

```bash
timeout 5 ./build/debug/src/endless_runner 2>&1 | grep -E "SDLRenderer|BeginFrame|EndFrame|destroyed"
```

Expected: SDLRenderer initialized, BeginFrame/EndFrame cycle, clean shutdown.

- [ ] **Step 4: Layer dependency check**

```bash
python3 scripts/check_layer_deps.py
```

Expected: `✅ All layer dependencies valid`
