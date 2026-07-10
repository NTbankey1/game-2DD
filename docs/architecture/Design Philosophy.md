---
title: Design Philosophy
date: 2026-07-04
tags:
  - architecture
  - design-principles
aliases:
  - Design Principles
  - Nguyên tắc thiết kế
---

# Design Philosophy

%% Core beliefs that drive every architecture decision in this project %%

## Vision

> **Một 2D endless runner** — clean code, kiến trúc rõ ràng, dễ mở rộng.
> Platformer cổ điển, procedural sprites, parallax background, difficulty scaling.

## MVP Definition

| Tính năng | MVP? | Ghi chú |
|-----------|------|---------|
| Player jump (coyote time, jump buffer) | ✅ | Core gameplay |
| Obstacle spawning, difficulty scaling | ✅ | Core gameplay |
| Score tracking, high score | ✅ | Retention |
| Menu → Gameplay → Pause → Game Over | ✅ | UX flow |
| Procedural sprites (GPU) | ✅ | Visual identity |
| Parallax background | ✅ | Visual depth |
| Audio SFX | ❌ | Phase 2 |
| Touch input | ❌ | Phase 2 |
| Leaderboard | ❌ | Phase 2+ |

## Core Principles

### 1. Engine-Game Separation ^engine-game-separation

```
engine/  → Reusable game-agnostic infrastructure
game/    → Endless-runner specific logic
```

Engine không biết game tồn tại. Game có thể replace engine backend. Xem [[Layer Architecture]].

### 2. ECS Over OOP ^ecs-over-oop

> [!example] Instead of:
> ```cpp
> class Player : public Entity { /* ... */ };
> class Obstacle : public Entity { /* ... */ };
> ```

> [!example] We do:
> ```cpp
> auto player = registry.create();
> registry.emplace<Transform>(player, ...);
> registry.emplace<Velocity>(player, ...);
> registry.emplace<PlayerTag>(player);
> ```

**Lý do:** Composition over inheritance. Một entity vừa có thể là player vừa có physics — không cần diamond inheritance. ECS cache-friendly hơn cho hot loop. Xem [[Gameplay Systems]].

### 3. Everything is Replaceable ^everything-is-replaceable

Mọi interface trong Engine đều có thể có nhiều implementation:

| Interface | Implementation(s) | Dùng khi nào |
|-----------|-------------------|--------------|
| `IRenderer` | `SDLRenderer`, `NullRenderer` | Production, testing, headless |
| `IInputDevice` | `SDLInputDevice`, `MockInputDevice` | Production, unit test |
| `IAudioDevice` | `SDLAudioDevice`, `NullAudioDevice` | Production, CI/test |

> [!info] Dependency Injection
> Game nhận interface qua constructor. CI không cần SDL3.
> ```cpp
> class Game {
>     IRenderer& m_renderer;
>     IInputDevice& m_input;
>     IAudioDevice& m_audio;
> };
> ```

### 4. Fail Fast

- ==Không silent error handling.== Nếu asset không load được, crash ngay lúc startup, không phải lúc gameplay.
- Assertions ở mọi nơi — đặc biệt là `dev_assert` có thể tắt trong release.
- [[Memory & Performance#fail-fast]] có danh sách cụ thể.

### 5. Zero Allocation in Hot Path

- ==Không `new`/`delete`, không `std::vector::push_back`, không `malloc` trong gameplay update loop.==
- Object pool cho obstacles và particles. Xem [[Memory & Performance#object-pool]].
- Fixed-size pools cho mọi thứ trong ECS registry.

### 6. Self-Documenting Identifiers

```cpp
// GOOD — rõ ràng
auto player = registry.create();
registry.emplace<PlayerTag>(player);
registry.emplace<Transform>(player, 100.0f, 300.0f);

// BAD — phải guess
auto e = r.create();
r.emplace<PlayerTag>(e);
```

---

## Key Tradeoffs

| Decision | Gain | Cost |
|----------|------|------|
| ECS over OOP | Composable, cache-friendly | Steeper learning curve |
| Engine/Game split | Reusable engine | Extra abstraction layer, more files |
| EventBus for cross-module | Decoupled modules | Async debugging harder |
| Fixed timestep | Deterministic physics | Extra interpolation code |
| Procedural sprites | Self-contained, no asset pipeline | Harder to add art |
| FetchContent | Reproducible builds | Slower first build |

> [!tip] Khi nào đánh đổi lại
> - Nếu gameplay cần thay đổi nhanh, handle-code trong Game layer, đừng vội generalize lên Engine.
> - Nếu thấy abstraction không pay off, hủy nó. Kiến trúc phục vụ development speed, không phải ngược lại.

---

## Related Notes
- [[Layer Architecture]] — how principles manifest in layers
- [[Design Patterns]] — specific patterns used
- [[Architecture Pitfalls]] — what happens when principles are violated

^design-philosophy
