---
title: Architecture Pitfalls
date: 2026-07-04
tags:
  - architecture
  - pitfalls
  - anti-patterns
aliases:
  - Anti-patterns
  - Things to avoid
  - Cạm bẫy kiến trúc
---

# Architecture Pitfalls

%% 10 điều cần tránh — đọc trước khi code, review trước khi merge %%

## 1. Layer Violation ^layer-violation

> **Không:** Code trong Core include Engine header
> **Không:** Engine include Game header
> **Không:** Game include Application header

```cpp
// 🔴 VIOLATION — Engine includes Game
// engine/physics/CollisionSystem.hpp:
#include <game/player/Player.hpp>  // LỖI! Engine → Game

// 🟢 CORRECT — Game includes Engine
// game/states/GameState.hpp:
#include <engine/scene/IScene.hpp>  // OK! Game → Engine
```

**Hậu quả:** Circular dependency, build graph rối, không test được từng layer.

---

## 2. OOP Entity Hierarchy ^oop-entity

> **Không:** `class Player : public Entity`, `class Obstacle : public Entity`

```cpp
// 🔴 VIOLATION — OOP hierarchy
class Entity { /* ... */ };
class Player : public Entity { /* ... */ };
class MovingObstacle : public Obstacle { /* ... */ };

// 🟢 CORRECT — ECS composition
registry.emplace<PlayerTag>(entity);
registry.emplace<ObstacleTag>(entity);
registry.emplace<Velocity>(entity);  // Thêm velocity cho moving obstacle
```

**Hậu quả:** Diamond inheritance, khó thêm behavior mới, cache miss.

---

## 3. Heap Allocation in Hot Path

> **Không:** `new`, `malloc`, `std::vector::push_back`, `std::make_unique` trong update loop

```cpp
// 🔴 VIOLATION — Heap allocation mỗi frame
void ObstacleSpawner::update(float dt) {
    m_obstacles.push_back(Obstacle{...});  // push_back có thể reallocate
}

// 🟢 CORRECT — Pre-allocated pool
void ObstacleSpawner::update(float dt) {
    auto& obj = m_pool.acquire();  // O(1), no allocation
    obj.reset(x, y);
}
```

**Hậu quả:** Micro-stutter, cache pollution, heap fragmentation.

---

## 4. Singleton for Everything

> **Không:** Global singleton cho mọi system
> **Chỉ dùng:** EventBus, Logger (infrastructure)

```cpp
// 🔴 VIOLATION — Game system as singleton
ScoreSystem::instance().addScore(100);     // Hidden dependency
PlayerSystem::instance().jump();            // Tight coupling

// 🟢 CORRECT — DI through constructor
m_scoreSystem.addScore(100);                // Explicit dependency
```

**Hậu quả:** Hidden dependencies, không testable, global state rối.

---

## 5. No Tests "Because It's Games"

> **Không:** "Game code thay đổi nhanh, không cần test"

[[Testing Strategy]] chứng minh: logic phức tạp nhất (physics, collision, scoring) đều test được.

```cpp
// 🟢 CORRECT — Test core gameplay logic
TEST_CASE("Jump velocity applied correctly", "[game][player]") {
    auto vel = getTestPlayerVelocity(afterJump);
    CHECK(vel.y == -JUMP_VELOCITY);
}
```

**Hậu quả:** Regression mỗi lần sửa. Bug đến production.

---

## 6. Premature Optimization ^premature-optimization

> **Không:** Tối ưu trước khi profile
> **Cần:** Viết clean trước, profile sau, optimize chỗ cần

```cpp
// 🔴 VIOLATION — Premature: unnecessarily complex
struct Transform {
    float pos[2];       // Instead of Vec2 — "for SIMD"
};                      // → Giảm readability, không có SIMD thật

// 🟢 CORRECT — Clean first
struct Transform {
    Vec2 pos;           // Readable, maintainable
};                      // → SIMD khi profile chứng minh cần
```

**Hậu quả:** Code phức tạp vô ích, khó maintain.

---

## 7. Hardcoded Magic Numbers

> **Không:** Numbers without name

```cpp
// 🔴 VIOLATION — Magic numbers
void PlayerSystem::update() {
    if (m_groundFrames < 5) canJump = true;  // 5 là gì?
    velocity.v.y = -450.0f;                    // 450 là gì?
}

// 🟢 CORRECT — Named constants
constexpr int COYOTE_FRAMES = 5;
constexpr float JUMP_VELOCITY = -450.0f;
```

**Hậu quả:** Không ai dám sửa số. Debug nightmare.

---

## 8. Dependency Hell ^dependency-hell

> **Không:** Thêm dependency cho 1 function
> **Cần:** Cân nhắc kỹ mỗi lần thêm thư viện

```cpp
// 🔴 VIOLATION — Thêm cả thư viện cho 1 utility
// Chỉ để dùng 1 hàm Vec3 cross product
// → FetchContent thêm cả GLM (~3MB)

// 🟢 CORRECT — Tự viết nếu đơn giản
constexpr Vec3 cross(const Vec3& a, const Vec3& b) { ... }
```

[[Third Party Libraries]] có decision tree.

**Hậu quả:** Build time tăng, security surface tăng, maintenance burden.

---

## 9. God Class / System

> **Không:** System làm quá nhiều việc

```cpp
// 🔴 VIOLATION — God system
class RenderSystem {
    void update() {
        drawSky();        // Background
        drawMountains();  // Parallax
        drawObstacles();  // Game objects
        drawPlayer();     // Player
        drawUI();         // Score, menus
        drawVignette();   // Effect
        drawParticles();  // VFX
    }
};

// 🟢 CORRECT — Separation of concerns
class BackgroundRenderer { ... };
class ParallaxRenderer  { ... };
class SpriteRenderer    { ... };
class UIRenderer        { ... };
```

**Hậu quả:** 500-line function. Không ai dám sửa. Không test được.

---

## 10. Ignoring Error States

> **Không:** Giả định không bao giờ lỗi

```cpp
// 🔴 VIOLATION — Assume always OK
SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
SDL_RenderTexture(renderer, tex, ...);  // Nếu tex là null → crash

// 🟢 CORRECT — Validate
SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
if (!tex) {
    FATAL("Failed to create player texture: %s", SDL_GetError());
}
```

**Hậu quả:** Crashes ở chỗ không ngờ, khó debug. ==Fail fast.==

---

## Pitfall Checklist for Code Review

Trước mỗi merge, check:

- [ ] Layer direction đúng? (Game → Engine → Core, không ngược)
- [ ] ECS, không OOP hierarchy?
- [ ] Zero heap allocation trong hot path?
- [ ] Singleton access hợp lý? (chỉ EventBus/Logger)
- [ ] Unit test cho logic mới?
- [ ] Không premature optimization?
- [ ] Constants named, không magic numbers?
- [ ] Không dependency kéo vào vô ích?
- [ ] System single responsibility?
- [ ] Error states handled (fail fast)?

---

## Related Notes
- [[Design Philosophy]] — principles that prevent these pitfalls
- [[Layer Architecture]] — correct layer usage
- [[Memory & Performance]] — hot path discipline
- [[Event System]] — proper event usage
- [[Testing Strategy]] — how to test against pitfalls

^architecture-pitfalls
