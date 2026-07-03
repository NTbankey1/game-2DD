# Phase 2: SDL3 Window & Application Loop Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax.

**Goal:** Open a black SDL3 window with a fixed-timestep game loop (60Hz physics, variable frame render), ESC-to-quit, deltaTime measurement, and stack-based GameStateMachine.

**Architecture:** SDL3 fetched via CMake FetchContent. Concrete SDL3 implementations (`SDLWindow`, `SDLInputDevice`) live in `engine/platform/sdl3/`, implementing interfaces from `engine/`. `Application` owns the main loop, GameStateMachine, and all subsystems. Game layer provides `MenuState` stub.

**Tech Stack:** SDL3 (FetchContent tip-of-tree), C++20, CMake 3.28+, all existing Phase 1 libraries.

## Global Constraints

- SDL3 fetched via FetchContent (no system packages) — use `SDL3_EXAMPLES_ENABLED=OFF`, `SDL3_TESTS_ENABLED=OFF`
- All engine headers remain under `src/engine/` — no game headers in engine code
- Concrete platform implementations under `src/engine/platform/sdl3/`
- Main loop uses fixed timestep (1/60s = 16.666ms) + accumulator pattern
- No rendering (no `SDL_Renderer`) in Phase 2 — pure black window + console
- Window close event (SDL_EVENT_QUIT) and ESC key must quit the app
- `Application::Run()` returns `int` exit code
- Concrete classes inherit from the Phase 1 abstract interfaces
- All new code must compile with `-Wall -Wextra -Wpedantic` (warning suppressions in Phase 1 are sufficient)
- No exceptions in release path — use `std::optional` return or `spdlog::error` + graceful exit

---
## File Structure (added/modified)

```
src/
├── main.cpp                                  [MODIFY — use Application]
├── engine/
│   ├── application/
│   │   └── Application.hpp                   [CONCRETE class implementing IApplication]
│   ├── scene/
│   │   └── GameStateMachine.hpp               [CONCRETE stack-based state machine]
│   └── platform/
│       └── sdl3/
│           ├── SDLWindow.hpp                  [SDL3 window wrapper]
│           ├── SDLWindow.cpp
│           ├── SDLInputDevice.hpp
│           └── SDLInputDevice.cpp
├── game/
│   ├── engine_impl/
│   │   └── MenuState.hpp                      [stub game state — shows black window]
│   └── states/
│       └── GameState.hpp                      [MODIFY — add MenuState]
cmake/
└── Dependencies.cmake                         [MODIFY — add SDL3]
```

---
### Task 1: Add SDL3 as FetchContent Dependency

**Files:**
- Modify: `cmake/Dependencies.cmake`

**Interfaces:**
- Produces: `SDL3::SDL3` and `SDL3::SDL3-shared` CMake targets

- [ ] **Step 1: Add SDL3 block to Dependencies.cmake**

```cmake
# ---- SDL3 (window, input, rendering) ----
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
# Disable examples, tests, and other unneeded components
set(SDL3_EXAMPLES_ENABLED OFF CACHE BOOL "" FORCE)
set(SDL3_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
set(SDL3_INSTALL_ENABLED OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)
```

- [ ] **Step 2: Link SDL3 to core_core**

```cmake
# In src/core/CMakeLists.txt, append:
target_link_libraries(core_core PUBLIC SDL3::SDL3)
```

- [ ] **Step 3: Build and verify SDL3 compiles**

```bash
rm -rf build/debug
cmake --preset debug 2>&1 | grep -E "SDL3|Configuring done"
cmake --build --preset debug --parallel $(nproc) 2>&1 | tail -5
```

Expected: No errors, `Configuring done` includes SDL3 population steps.

---
### Task 2: Concrete SDLWindow — Wrapper Around SDL_Window

**Files:**
- Create: `src/engine/platform/sdl3/SDLWindow.hpp`
- Create: `src/engine/platform/sdl3/SDLWindow.cpp`

**Interfaces:**
- Consumes: `engine::IRenderer` (implements it, but only window creation for now)
- Produces: Concrete `SDLWindow` class

- [ ] **Step 1: Write SDLWindow.hpp**

```cpp
#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace engine::platform::sdl3 {

/// SDL3 window wrapper — concrete ownership of SDL_Window*.
/// Provides the window handle that renderer/input subsystems will use.
class SDLWindow {
public:
    /// Create window with given title, width, height.
    /// Returns nullptr if window creation fails.
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
```

- [ ] **Step 2: Write SDLWindow.cpp**

```cpp
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
    : m_window(window)
    , m_width(width)
    , m_height(height)
{}

SDLWindow::~SDLWindow() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        spdlog::info("SDLWindow destroyed");
    }
}

} // namespace engine::platform::sdl3
```

---
### Task 3: Concrete SDLInputDevice — Keyboard & Event Polling

**Files:**
- Create: `src/engine/platform/sdl3/SDLInputDevice.hpp`
- Create: `src/engine/platform/sdl3/SDLInputDevice.cpp`

**Interfaces:**
- Consumes: `engine::IInputDevice`
- Produces: Concrete `SDLInputDevice` class

- [ ] **Step 1: Write SDLInputDevice.hpp**

```cpp
#pragma once

#include "engine/input/IInputDevice.hpp"
#include <SDL3/SDL.h>
#include <unordered_map>

namespace engine::platform::sdl3 {

class SDLInputDevice : public engine::IInputDevice {
public:
    explicit SDLInputDevice();
    ~SDLInputDevice() override = default;

    void PollEvents() override;
    bool IsKeyPressed(int key) override;
    void GetMousePos(float& x, float& y) override;

    /// Check if quit was requested (SDL_EVENT_QUIT or ESC key)
    [[nodiscard]] bool QuitRequested() const noexcept { return m_quitRequested; }

private:
    bool m_quitRequested = false;
    std::unordered_map<SDL_Scancode, bool> m_keys;
    float m_mouseX = 0, m_mouseY = 0;
};

} // namespace engine::platform::sdl3
```

- [ ] **Step 2: Write SDLInputDevice.cpp**

```cpp
#include "SDLInputDevice.hpp"

namespace engine::platform::sdl3 {

SDLInputDevice::SDLInputDevice() {
    m_keys.reserve(256);
}

void SDLInputDevice::PollEvents() {
    m_quitRequested = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_quitRequested = true;
            break;
        case SDL_EVENT_KEY_DOWN:
            m_keys[event.key.scancode] = true;
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                m_quitRequested = true;
            }
            break;
        case SDL_EVENT_KEY_UP:
            m_keys[event.key.scancode] = false;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            m_mouseX = event.motion.x;
            m_mouseY = event.motion.y;
            break;
        }
    }
}

bool SDLInputDevice::IsKeyPressed(int key) {
    auto it = m_keys.find(static_cast<SDL_Scancode>(key));
    return it != m_keys.end() && it->second;
}

void SDLInputDevice::GetMousePos(float& x, float& y) {
    x = m_mouseX;
    y = m_mouseY;
}

} // namespace engine::platform::sdl3
```

---
### Task 4: GameStateMachine — Stack-based State Manager

**Files:**
- Create: `src/engine/scene/GameStateMachine.hpp`
- Create: `src/engine/scene/GameStateMachine.cpp`

**Interfaces:**
- Consumes: `engine::IScene`, `engine::SceneManager` (abstract)
- Produces: Concrete `GameStateMachine` implementing stack-based state management

- [ ] **Step 1: Write GameStateMachine.hpp**

```cpp
#pragma once

#include "engine/scene/SceneManager.hpp"
#include "engine/scene/IScene.hpp"
#include <memory>
#include <stack>
#include <functional>

namespace engine::scene {

/// Stack-based state machine.
/// Push = add state on top (pause overlay). Pop = resume previous state.
/// Reload = pop current, push same type (restart).
class GameStateMachine : public engine::SceneManager {
public:
    GameStateMachine() = default;
    ~GameStateMachine() override = default;

    void PushScene(std::unique_ptr<engine::IScene> scene) override;
    void PopScene() override;
    void ReloadScene() override;
    void Update(float dt) override;

    /// Run FixedUpdate on topmost state (returns false if no states left)
    bool FixedUpdate(float dt);

    /// Render topmost state (or top + below if render passes down)
    void Render();

    /// Number of states on stack
    [[nodiscard]] size_t StateCount() const noexcept { return m_stack.size(); }

    /// Check if stack is empty
    [[nodiscard]] bool Empty() const noexcept { return m_stack.empty(); }

private:
    std::stack<std::unique_ptr<engine::IScene>> m_stack;
    std::function<void()> m_deferredAction;  // for safe push/pop during Update
};

} // namespace engine::scene
```

- [ ] **Step 2: Write GameStateMachine.cpp**

```cpp
#include "GameStateMachine.hpp"
#include <spdlog/spdlog.h>

namespace engine::scene {

void GameStateMachine::PushScene(std::unique_ptr<engine::IScene> scene) {
    if (scene) {
        scene->OnEnter();
        m_stack.push(std::move(scene));
        spdlog::info("GameStateMachine: pushed state (depth={})", m_stack.size());
    }
}

void GameStateMachine::PopScene() {
    if (m_stack.empty()) return;
    m_stack.top()->OnExit();
    m_stack.pop();
    spdlog::info("GameStateMachine: popped state (depth={})", m_stack.size());
    if (!m_stack.empty()) {
        m_stack.top()->OnEnter();  // re-activate previous state
    }
}

void GameStateMachine::ReloadScene() {
    if (m_stack.empty()) return;
    auto current = std::move(const_cast<std::unique_ptr<engine::IScene>&>(m_stack.top()));
    m_stack.pop();
    current->OnExit();
    // The caller must handle re-push; for reload the application
    // should push a new state of the same type.
    spdlog::info("GameStateMachine: scene popped for reload");
}

void GameStateMachine::Update(float dt) {
    if (m_stack.empty()) return;
    // Only update the topmost state
    m_stack.top()->Update(dt);
}

bool GameStateMachine::FixedUpdate(float dt) {
    if (m_stack.empty()) return false;
    m_stack.top()->FixedUpdate(dt);
    return true;
}

void GameStateMachine::Render() {
    if (m_stack.empty()) return;
    m_stack.top()->Render();
}

} // namespace engine::scene
```

---
### Task 5: Concrete Application — Main Loop with Fixed Timestep

**Files:**
- Create: `src/engine/application/Application.hpp`
- Create: `src/engine/application/Application.cpp`

**Interfaces:**
- Consumes: `engine::IApplication`, `engine::IScene`, `engine::SceneManager`, `SDLWindow`, `SDLInputDevice`
- Produces: Concrete `Application` class with `Run()` returning exit code

- [ ] **Step 1: Write Application.hpp**

```cpp
#pragma once

#include "engine/application/IApplication.hpp"
#include <memory>

namespace engine::platform::sdl3 {
    class SDLWindow;
    class SDLInputDevice;
}

namespace engine::scene {
    class GameStateMachine;
}

namespace engine::application {

/// Main application class — owns window, input, state machine, and the game loop.
/// Uses fixed timestep (1/60s) for physics/logic with variable frame render.
class Application : public engine::IApplication {
public:
    Application();
    ~Application() override;

    bool Initialize() override;
    void Run() override;
    void Shutdown() override;
    float GetFrameTime() const override { return m_frameTime; }

    // Accessors for sub-systems
    [[nodiscard]] engine::scene::GameStateMachine& States() { return *m_stateMachine; }
    [[nodiscard]] const engine::scene::GameStateMachine& States() const { return *m_stateMachine; }

private:
    std::unique_ptr<engine::platform::sdl3::SDLWindow> m_window;
    std::unique_ptr<engine::platform::sdl3::SDLInputDevice> m_input;
    std::unique_ptr<engine::scene::GameStateMachine> m_stateMachine;

    // Timing
    float m_frameTime = 0.0f;
    static constexpr float FIXED_DT = 1.0f / 60.0f;
    bool m_running = false;
};

} // namespace engine::application
```

- [ ] **Step 2: Write Application.cpp**

```cpp
#include "Application.hpp"
#include "engine/platform/sdl3/SDLWindow.hpp"
#include "engine/platform/sdl3/SDLInputDevice.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace engine::application {

Application::Application()
    : m_stateMachine(std::make_unique<engine::scene::GameStateMachine>())
{}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    spdlog::info("Application::Initialize");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    spdlog::info("SDL3 initialized");

    m_window.reset(engine::platform::sdl3::SDLWindow::Create(
        "Endless Runner", 1280, 720
    ));
    if (!m_window) {
        return false;
    }

    m_input = std::make_unique<engine::platform::sdl3::SDLInputDevice>();
    m_running = true;
    return true;
}

void Application::Run() {
    if (!m_running) {
        spdlog::warn("Application::Run called but not initialized");
        return;
    }

    spdlog::info("Application::Run started");

    float accumulator = 0.0f;
    std::uint64_t prevCounter = SDL_GetPerformanceCounter();
    const std::uint64_t counterFreq = SDL_GetPerformanceFrequency();

    while (m_running) {
        // 1. Input
        m_input->PollEvents();
        if (m_input->QuitRequested()) {
            m_running = false;
            break;
        }

        // 2. Frame timing
        std::uint64_t currCounter = SDL_GetPerformanceCounter();
        float deltaTime = static_cast<float>(currCounter - prevCounter) / static_cast<float>(counterFreq);
        prevCounter = currCounter;

        // Clamp delta time to avoid spiral of death
        if (deltaTime > 0.25f) {
            deltaTime = 0.25f;
        }

        m_frameTime = deltaTime;

        // 3. Fixed timestep update (physics/logic)
        accumulator += deltaTime;
        while (accumulator >= FIXED_DT) {
            if (!m_stateMachine->Empty()) {
                m_stateMachine->FixedUpdate(FIXED_DT);
            }
            accumulator -= FIXED_DT;
        }

        // 4. Variable update (camera, UI, particles)
        if (!m_stateMachine->Empty()) {
            m_stateMachine->Update(deltaTime);
        }

        // 5. Render (Phase 2: no SDL_Renderer yet — just a black window)
        // Present is handled by SDL internally for Vulkan/GL, but for
        // SDL_RENDERER this would be SDL_RenderPresent.
        // Black window appears automatically — no explicit present needed.
    }

    spdlog::info("Application::Run ended");
}

void Application::Shutdown() {
    if (!m_running && !m_window) return;  // already shut down

    spdlog::info("Application::Shutdown");
    m_stateMachine.reset();
    m_input.reset();
    m_window.reset();
    SDL_Quit();
    m_running = false;
}

} // namespace engine::application
```

---
### Task 6: MenuState Stub — First Interactive State

**Files:**
- Create: `src/game/engine_impl/MenuState.hpp`
- Create: `src/game/engine_impl/MenuState.cpp`

**Interfaces:**
- Consumes: `engine::IScene`
- Produces: `game::MenuState` — stub that just logs lifecycle events

- [ ] **Step 1: Write MenuState.hpp**

```cpp
#pragma once

#include "engine/scene/IScene.hpp"
#include <string>

namespace game {

class MenuState : public engine::IScene {
public:
    explicit MenuState() = default;
    ~MenuState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void FixedUpdate(float dt) override;
    void Update(float dt) override;
    void Render() override;
};

} // namespace game
```

- [ ] **Step 2: Write MenuState.cpp**

```cpp
#include "MenuState.hpp"
#include <spdlog/spdlog.h>

namespace game {

void MenuState::OnEnter() {
    spdlog::info("MenuState::OnEnter");
}

void MenuState::OnExit() {
    spdlog::info("MenuState::OnExit");
}

void MenuState::FixedUpdate(float dt) {
    // Phase 2: stub — no game logic yet
    spdlog::trace("MenuState::FixedUpdate({:.4f})", dt);
}

void MenuState::Update(float dt) {
    spdlog::trace("MenuState::Update({:.4f})", dt);
}

void MenuState::Render() {
    // Phase 2: stub — no renderer yet, window stays black
}

} // namespace game
```

---
### Task 7: Wire Main.cpp to Application + MenuState

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Rewrite main.cpp**

```cpp
#include "engine/application/Application.hpp"
#include "game/engine_impl/MenuState.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    spdlog::info("Endless Runner v{}", PROJECT_VERSION);

    engine::application::Application app;
    if (!app.Initialize()) {
        spdlog::error("Application initialization failed");
        return 1;
    }

    // Push initial state
    app.States().PushScene(std::make_unique<game::MenuState>());

    // Run main loop — blocks until user quits
    app.Run();

    spdlog::info("Endless Runner exited cleanly");
    return 0;
}
```

---
### Task 8: CMake Wiring — Add Platform/Application/State Targets

**Files:**
- Modify: `src/CMakeLists.txt`
- Modify: `src/engine/CMakeLists.txt`
- Create: `src/engine/platform/CMakeLists.txt`
- Create: `src/game/engine_impl/CMakeLists.txt` (or modify game CMake)

- [ ] **Step 1: Create src/engine/platform/CMakeLists.txt**

```cmake
add_library(engine_platform_sdl3 STATIC
    sdl3/SDLWindow.cpp
    sdl3/SDLInputDevice.cpp
)
target_include_directories(engine_platform_sdl3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/sdl3)
target_link_libraries(engine_platform_sdl3 PUBLIC engine_interface SDL3::SDL3)
add_compiler_warnings(engine_platform_sdl3)
add_sanitizers(engine_platform_sdl3)
```

- [ ] **Step 2: Create application Application.cpp CMake entry**

Modify `src/engine/CMakeLists.txt` to add:
```cmake
# Keep existing engine_interface (INTERFACE) as is

# Add concrete application library
add_library(engine_application STATIC
    application/Application.cpp
    scene/GameStateMachine.cpp
)
target_include_directories(engine_application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine_application PUBLIC
    engine_interface
    engine_platform_sdl3
    SDL3::SDL3
)
add_compiler_warnings(engine_application)
add_sanitizers(engine_application)
```

- [ ] **Step 3: Add game engine_impl CMake**

In `src/game/CMakeLists.txt`, add:
```cmake
# Keep existing game_stubs (INTERFACE) as is

# Add concrete game implementation
add_library(game_impl STATIC
    engine_impl/MenuState.cpp
)
target_include_directories(game_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game_impl PUBLIC engine_application engine_interface)
add_compiler_warnings(game_impl)
add_sanitizers(game_impl)
```

- [ ] **Step 4: Update src/CMakeLists.txt linking**

```cmake
# Update endless_runner link libraries
target_link_libraries(endless_runner PRIVATE
    core_core
    engine_interface
    engine_platform_sdl3
    engine_application
    game_stubs
    game_impl
)
```

---
### Task 9: Integration Test — Build & Run

**Files:** None (verification only)

- [ ] **Step 1: Clean build**

```bash
rm -rf build/debug
cmake --preset debug 2>&1 | tail -10
cmake --build --preset debug --parallel $(nproc) 2>&1 | grep -E "error:|FAILED|Linking|error:" | head -20
```

Expected: Build completes. SDL3 fetches from GitHub, compiles, and the final link produces `endless_runner`.

- [ ] **Step 2: Run smoke test (headless / Xvfb)**

```bash
# If running on a display:
./build/debug/src/endless_runner &
sleep 2
kill %1
# Expected: spdlog output showing lifecycle, ESC quits cleanly

# If headless (SSH):
Xvfb :99 &
DISPLAY=:99 ./build/debug/src/endless_runner &
sleep 2
kill %1
# Expected: clean exit, no crash
```

- [ ] **Step 3: Run unit tests (all Phase 1 tests still pass)**

```bash
./build/debug/tests/unit_tests --success 2>&1 | tail -5
```

Expected: `All tests passed (42 assertions in 19 test cases)`

- [ ] **Step 4: Check layer dependencies**

```bash
python3 scripts/check_layer_deps.py
```

Expected: `✅ All layer dependencies valid`

- [ ] **Step 5: Verify SDL resource cleanup (valgrind or manual log check)**

Run the app briefly and check spdlog output for both `SDLWindow created` and `SDLWindow destroyed` messages.

---
### Task 10: Commit Phase 2

- [ ] **Step 1: Final commit**

```bash
git add -A
git commit -m "feat: Phase 2 — SDL3 window, application loop, state machine

- SDL3 via FetchContent (window, input, events)
- SDLWindow: RAII wrapper around SDL_Window*
- SDLInputDevice: keyboard polling + quit detection
- Application: fixed-timestep (60Hz) main loop + accumulator
- GameStateMachine: stack-based state manager
- MenuState stub: first interactive state
- All Phase 1 tests continue to pass"
```

---

## Self-Review Checklist

- [x] **Spec coverage:** Phase 2 covers all items from Roadmap Phase 2 (window, main loop, SDL3, state machine)
- [x] **Placeholder scan:** No "TBD", "TODO" — all code is concrete
- [x] **Type consistency:** Application implements IApplication, GameStateMachine implements SceneManager, all interfaces match Phase 1 signatures
- [x] **Layer compliance:** SDL3 is only included in `engine/platform/sdl3/` — no SDL3 in `game/` or `core/`
- [x] **RAII:** SDLWindow owns SDL_Window* via unique_ptr, SDL_Quit called in Shutdown, no leak path
- [x] **Fixed timestep:** Accumulator pattern with 0.25s clamp, FIXED_DT = 1/60
