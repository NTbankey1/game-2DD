# Phase 4: Input System — Command Pattern & Event-Driven Input

**Goal:** Press a key → `InputCommand` published via EventBus → GameState reacts.

## Global Constraints
- No SDL3 headers in `game/` or `core/`
- InputMapper is a standalone class (testable without SDL)
- All key presses → `InputCommand` on EventBus, not direct function calls

---
### Task 1: InputCommand Event + InputMapper

**Files:**
- Create: `src/game/input/InputCommand.hpp`
- Create: `src/game/input/InputMapper.hpp`
- Create: `src/game/input/InputMapper.cpp`

```cpp
// InputCommand.hpp
#pragma once
#include "core/events/IEvent.hpp"
#include "core/math/Vec2.hpp"
#include <cstdint>

namespace game {
enum class InputAction : uint8_t { Jump, Slide, Pause, Confirm, Cancel, None };
struct InputCommand : core::events::IEvent {
    InputAction action = InputAction::None;
};
} // namespace game
```

```cpp
// InputMapper.hpp
#pragma once
#include "game/input/InputCommand.hpp"
#include <SDL3/SDL.h>
#include <unordered_map>

namespace game {
class InputMapper {
public:
    InputMapper();

    InputAction MapKey(SDL_Scancode scancode) const;
    void Bind(SDL_Scancode scancode, InputAction action);
    void Unbind(SDL_Scancode scancode);

private:
    std::unordered_map<SDL_Scancode, InputAction> m_bindings;
};
} // namespace game
```

File: `src/engine/input/IInputDevice.hpp` — already has `PollEvents(), IsKeyPressed(), GetMousePos()`.

---
### Task 2: Wire into Application — PollEvents → EventBus

**Modify:** `src/engine/application/Application.cpp`
- After `m_input->PollEvents()`, iterate active keys → forward to EventBus as `InputCommand`
- Keep existing `QuitRequested()` path

**Modify:** `src/game/engine_impl/MenuState.cpp`
- Subscribe to `InputCommand` on EventBus
- Log which action received (Jump/Slide/Pause/Confirm/Cancel)

---
### Implementation

Implement inline now.
