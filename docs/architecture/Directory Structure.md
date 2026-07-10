---
title: Directory Structure
date: 2026-07-04
tags:
  - architecture
  - project-structure
aliases:
  - File Layout
  - Cấu trúc thư mục
---

# Directory Structure

%% Full project file tree — biết đặt code mới ở đâu và tìm file ở đâu %%

## Root

```tree
game/
├── CMakeLists.txt              # Root build definition
├── CMakePresets.json            # Build presets (debug/release/ci)
├── .clang-format               # Code formatting rules
├── .gitignore
├── cmake/
│   ├── CompilerWarnings.cmake  # -Wall -Wextra -Wpedantic config
│   ├── Dependencies.cmake      # FetchContent declarations
│   ├── Packaging.cmake         # CPack config
│   └── Sanitizers.cmake        # ASan/UBSan setup
├── docs/                       # Architecture docs (Obsidian vault)
│   ├── ADR/                    # Architecture Decision Records
│   ├── CodingStandard.md
│   ├── game-architecture-overview.md
│   ├── Skills-Overview.md
│   └── architecture/           # Sub-notes (this section)
├── skills/                     # Claude Code skill definitions
│   └── Skills Ecosystem.canvas
├── scripts/                    # Build/dev helper scripts
│   ├── format.sh
│   ├── setup.sh
│   └── test.sh
├── src/                        # Source code
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── core/                   # Layer 0: Core
│   ├── engine/                 # Layer 1: Engine
│   ├── game/                   # Layer 2: Game
│   └── Application/            # Layer 3: Application
└── tests/                      # Test suite
    ├── CMakeLists.txt
    └── unit/                   # Catch2 unit tests
```

---

## Source Tree Detail

### Core — `/src/core/`

```
core/
├── CMakeLists.txt          # Static library build
├── core.hpp                # Umbrella include
├── math/
│   ├── Math.hpp            # Constants & utilities
│   ├── Vec2.hpp            # 2D vector (GLM wrapper)
│   └── Rect.hpp            # Axis-aligned bounding box
├── ecs/
│   ├── EcsFwd.hpp          # Forward declarations
│   └── Registry.hpp        # EnTT registry wrapper
├── events/
│   ├── IEvent.hpp          # Event interface
│   └── EventBus.hpp        # Publish/subscribe
└── config/
    └── ConfigTypes.hpp     # Game configuration structs
```

### Engine — `/src/engine/`

```
engine/
├── engine.hpp              # Umbrella include
├── application/
│   ├── IApplication.hpp    # Application interface
│   └── Application.hpp/cpp # Concrete implementation
├── input/
│   ├── IInputDevice.hpp    # Input device interface
│   └── KeyEvent.hpp        # Key event types
├── renderer/
│   ├── IRenderer.hpp       # Renderer interface
│   ├── RenderComponents.hpp# Render ECS components
│   ├── RenderSystem.hpp/cpp# Render system logic
│   ├── BitmapFont.hpp      # Bitmap font definitions
│   └── TextRenderer.hpp/cpp# Text rendering
├── physics/
│   ├── PhysicsComponents.hpp# Physics ECS components
│   ├── PhysicsSystem.hpp/cpp
│   └── CollisionSystem.hpp/cpp
├── audio/
│   └── IAudioDevice.hpp    # Audio interface
├── scene/
│   ├── IScene.hpp          # Scene interface
│   ├── SceneManager.hpp    # Scene stack
│   └── GameStateMachine.hpp/cpp
├── camera/
│   └── Camera.hpp          # Camera/viewport
├── resource/
│   └── IAssetLoader.hpp    # Asset loading interface
└── platform/sdl3/          # SDL3 implementations
    ├── SDLWindow.hpp/cpp
    ├── SDLInputDevice.hpp/cpp
    └── SDLRenderer.hpp/cpp
```

### Game — `/src/game/`

```
game/
├── game.hpp                # Umbrella include
├── player/
│   └── Player.hpp          # Player ECS setup + logic
├── obstacles/
│   └── Obstacle.hpp        # Obstacle spawning + logic
├── scoring/
│   └── ScoreSystem.hpp     # Score + high score
├── input/
│   ├── InputCommand.hpp    # Command pattern definitions
│   ├── InputMapper.hpp/cpp # Key → Command mapping
├── engine_impl/            # Engine interface implementations
│   ├── MenuState.hpp/cpp
│   └── GameState.hpp       # Main gameplay state
└── states/
    └── GameState.hpp       # State machine states
```

---

## Tests

```
tests/
├── CMakeLists.txt          # CTest discovery
└── unit/
    ├── core/               # Core layer unit tests
    │   ├── math/
    │   ├── ecs/
    │   └── events/
    ├── engine/             # Engine system tests
    │   ├── physics/
    │   └── renderer/
    └── game/               # Game logic tests
        ├── player/
        └── scoring/
```

---

## Quy Tắc Đặt File Mới

1. **Theo layer:** Nếu code thuộc Core, đặt trong `src/core/`. Nếu là logic game, đặt trong `src/game/`.
2. **Theo feature, không theo pattern:** `game/player/Player.hpp` — không `game/systems/PlayerSystem.hpp`.
3. **Header cho interface, .cpp cho implementation:** Interface trong Engine là header-only. SDL3 implementation có `.cpp`.
4. **Test mirror source:** `tests/unit/core/math/Vec2Test.cpp` test `src/core/math/Vec2.hpp`.

---

## Related Notes
- [[Layer Architecture]] — understand what goes in each layer
- [[Design Philosophy]] — principles behind this separation
- [[Architecture Pitfalls#layer-violation]] — what not to do

^directory-structure
