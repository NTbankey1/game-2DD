---
title: Design Patterns Catalog
date: 2026-07-04
tags:
  - architecture
  - design-patterns
aliases:
  - Patterns
  - Design Patterns
---

# Design Patterns Catalog

%% Danh sách design patterns được dùng — ai dùng, ở đâu, tại sao %%

## 1. Observer (Event Bus) ^1-observer-event-bus

**Dùng ở:** `core/events/EventBus.hpp`
**Cho:** Giao tiếp liên-module, decouple publisher/subscriber.

> [!example] Pattern
> ```cpp
> // Publish
> EventBus::publish(PlayerDeadEvent{reason});
> 
> // Subscribe
> EventBus::subscribe<PlayerDeadEvent>([](const auto& e) {
>     audio.playDeathSfx();
> });
> ```

**Dùng để:** CollisionSystem không gọi trực tiếp AudioSystem. Nó publish event, ai subscribe thì react. Xem [[Event System]].

---

## 2. State Pattern

**Dùng ở:** `engine/scene/GameStateMachine.hpp`, `game/states/GameState.hpp`
**Cho:** Quản lý game state: Menu → Gameplay → Pause → Game Over.

```cpp
class IScene {
    virtual void enter() = 0;
    virtual void update(float dt) = 0;
    virtual void render() = 0;
    virtual void exit() = 0;
};
```

[[Runtime Flow#state-machine]] có diagram chi tiết.

---

## 3. Command Pattern ^3-command-pattern

**Dùng ở:** `game/input/InputCommand.hpp`, `InputMapper`
**Cho:** Tách input handling khỏi game logic. Cho phép rebind keys, macro, replay.

```cpp
class ICommand {
    virtual void execute(Registry& reg) = 0;
};

class JumpCommand : public ICommand {
    void execute(Registry& reg) override {
        // Tìm player entity, apply velocity
    }
};

// InputMapper maps keys to commands
std::unordered_map<Key, std::unique_ptr<ICommand>> m_bindings;
```

---

## 4. Dependency Injection

**Dùng ở:** Mọi constructor trong Game Layer.
**Cho:** Testability (mock implementation), replaceable backend.

```cpp
class Game {
    Game(IRenderer& r, IInputDevice& i, IAudioDevice& a, Registry& reg);
};
```

Xem [[Design Philosophy#everything-is-replaceable]].

---

## 5. ECS Pattern (Entity-Component-System)

**Dùng ở:** `core/ecs/` — EnTT wrapper.
**Cho:** Composition over inheritance. Cache-friendly iteration.

```cpp
// System iterates over components, not entities
registry.view<Transform, Velocity>().each([](auto& t, auto& v) {
    t.pos += v * dt;
});
```

Xem [[Design Philosophy#ecs-over-oop]].

---

## 6. Object Pool

**Dùng ở:** Obstacle Spawner, Particle System.
**Cho:** Tránh heap allocation trong hot loop.

```cpp
template<typename T>
class ObjectPool {
    std::vector<T> m_pool;
    std::vector<size_t> m_freeIndices;  // Stack of free slots

    T& acquire() { /* pop from free stack */ }
    void release(T& obj) { /* push back to free stack */ }
};
```

Xem [[Memory & Performance#object-pool]].

---

## 7. Singleton (controlled)

**Dùng ở:** `EventBus`, `Logger`.
**Cho:** Global access point — ==chỉ dùng cho infrastructure, không dùng cho game logic==.

> [!warning] Singleton Anti-pattern
> ==Không singleton cho system game.== `ScoreSystem` không phải singleton. Nhận qua DI. Singleton dẫn đến hidden dependency, khó test.

---

## 8. Strategy Pattern

**Dùng ở:** `DifficultyManager`
**Cho:** Thay đổi hành vi difficulty scaling runtime — configurable difficulty curve.

```cpp
class DifficultyStrategy {
    virtual float getSpawnInterval(float elapsed) = 0;
    virtual float getSpeedMultiplier(float elapsed) = 0;
};
class LinearDifficulty : DifficultyStrategy { /* ... */ };
class ExponentialDifficulty : DifficultyStrategy { /* ... */ };
```

---

## 9. Component Pattern (Game UI)

**Dùng ở:** UI elements in Game Layer.
**Cho:** Tái sử dụng UI logic.

```cpp
class UIComponent {
    virtual void render(IRenderer&) = 0;
    virtual bool handleInput(const InputEvent&) = 0;
};
class ScoreDisplay : UIComponent { /* ... */ };
class MenuButton : UIComponent { /* ... */ };
```

---

## Pattern Use Heatmap

| Pattern | Frequency | Hot path? | Notes |
|---------|-----------|-----------|-------|
| ECS Pattern | ⭐⭐⭐⭐⭐ | ✅ Every frame | Engine core |
| Observer (EventBus) | ⭐⭐⭐⭐ | ✅ Per event | Cross-module comms |
| State Pattern | ⭐⭐⭐ | ✅ State transitions | GameStateMachine |
| Command Pattern | ⭐⭐⭐ | ✅ Per input | InputMapper |
| Dependency Injection | ⭐⭐⭐⭐ | ❌ Setup only | Constructors |
| Object Pool | ⭐⭐⭐ | ✅ Every frame | Obstacles, particles |
| Singleton | ⭐ | ❌ | EventBus, Logger only |
| Strategy | ⭐⭐ | ✅ Per tick | Difficulty |
| Component (UI) | ⭐⭐ | ❌ | Menu, HUD |

---

## Related Notes
- [[Design Philosophy]] — principles behind patterns
- [[Event System]] — Observer pattern in detail
- [[Gameplay Systems]] — patterns in action
- [[Architecture Pitfalls]] — pattern misuse

^design-patterns
