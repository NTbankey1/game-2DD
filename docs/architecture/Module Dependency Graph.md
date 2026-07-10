---
title: Module Dependency Graph
date: 2026-07-04
tags:
  - architecture
  - dependencies
aliases:
  - Dependency Graph
  - Module Dependencies
---

# Module Dependency Graph

%% Chi tiết dependency giữa các module — biết module A có phụ thuộc module B không %%

## Full Dependency Graph

```mermaid
graph LR
    subgraph Application
        main["main.cpp"]
        app["Application"]
    end
    subgraph Game
        gs["GameState"]
        ms["MenuState"]
        ps["Player System"]
        os["Obstacle System"]
        ss["Score System"]
        im["InputMapper"]
    end
    subgraph Engine
        rdr["RenderSystem"]
        phy["PhysicsSystem"]
        col["CollisionSystem"]
        tex["TextRenderer"]
        sdl_w["SDLWindow"]
        sdl_r["SDLRenderer"]
        sdl_i["SDLInputDevice"]
        cam["Camera"]
    end
    subgraph Core
        ecs["ECS Wrapper"]
        eb["EventBus"]
        vec["Vec2"]
        rect["Rect"]
        cfg["Config"]
    end
    subgraph External
        entt["EnTT"]
        glm["GLM"]
        spd["spdlog"]
        sdl3["SDL3"]
    end

    %% Application
    main --> app
    app --> gs
    app --> sdl_w

    %% Game → Engine
    gs --> rdr
    gs --> phy
    gs --> ps
    gs --> os
    gs --> ss
    ms --> tex
    ps --> col
    os --> col
    im --> sdl_i

    %% Engine → Core
    rdr --> ecs
    rdr --> vec
    phy --> ecs
    phy --> vec
    col --> ecs
    col --> rect
    tex --> vec
    cam --> vec

    %% Game → Core
    ps --> ecs
    ps --> eb
    os --> ecs
    ss --> eb
    ss --> cfg

    %% Core → External
    ecs --> entt
    vec --> glm
    rect --> glm
    eb --> spd

    %% Engine → External
    sdl_w --> sdl3
    sdl_r --> sdl3
    sdl_i --> sdl3
```

> [!info] Quy tắc
> Mũi tên = "phụ thuộc vào". Dependency direction: Application → Game → Engine → Core → External.

---

## Include Map

Layer nào include được layer nào:

```
        Can include → | External | Core | Engine | Game | Application
Include ↓             |          |      |        |      |
──────────────────────┼──────────┼──────┼────────┼──────┼────────────
External              │    ✓     |  ❌  |   ❌   |  ❌  |     ❌
Core                  │    ✓     |  ✓   |   ❌   |  ❌  |     ❌
Engine                │    ✓     |  ✓   |   ✓    |  ❌  |     ❌
Game                  │    ✓     |  ✓   |   ✓    |  ✓   |     ❌
Application           │    ✓     |  ✓   |   ✓    |  ✓   |     ✓
```

---

## Critical Dependency Paths

### Player Jump (Game → Engine → Core)

```
JumpCommand (Game)
  → Velocity updated (Game)
    → PhysicsSystem (Engine)
      → Transform updated (Engine)
        → Vec2 operations (Core)
          → GLM
```

### Collision (Engine → Core)

```
CollisionSystem (Engine)
  → Rect::intersects (Core)
    → Vec2 comparisons (Core)
      → GLM
```

### Render Frame (Full Stack)

```
main.cpp → Application::run
  → GameState::update → RenderSystem::render
    → IRenderer::beginFrame (→ SDLRenderer)
    → ECS view iteration (→ EnTT)
    → Vec2→SDL_FPoint conversion (→ GLM/SDL3)
    → IRenderer::endFrame (→ SDLRenderer)
```

---

## Circular Dependency Detection

Liệt kê include trong project:

```bash
grep -rn '#include "\.\.' src/ --include="*.hpp" | \
    grep -v 'test_' | \
    awk '{print $2}' | sort | uniq
```

Nếu thấy pattern này, đó là circular dependency:

```
engine/PhysicsSystem.hpp → game/Player.hpp
game/Player.hpp → engine/PhysicsSystem.hpp  ← CIRCULAR!
```

> [!warning] Circular Dependencies
> Mỗi khi phát hiện circular dependency, ==dừng lại và refactor.==
> Giải pháp: [[Event System]] hoặc [[Design Patterns#3-command-pattern|Command Pattern]].

---

## What If I Need to Cross Layers?

Luồng chính xác:

```
You want:  Obstacle dies → Audio plays
Don't:     ObstacleSystem calls AudioSystem directly
Instead:   1. ObstacleSystem publishes ObstacleDestroyedEvent
           2. AudioSystem subscribes and plays sound
```

[[Event System]] có danh sách events và flow.

---

## Related Notes
- [[Layer Architecture]] — layer rules
- [[Event System]] — cross-module communication
- [[Third Party Libraries]] — external dependency details
- [[Architecture Pitfalls#layer-violation]] — what not to do

^module-dependency-graph
