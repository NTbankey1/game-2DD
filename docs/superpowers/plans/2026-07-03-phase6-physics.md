# Phase 6: Physics & Collision System Implementation Plan

**Goal:** Player falls with gravity, lands on ground plane, AABB collision detection, collision events published to EventBus.

---
### Task 1: PhysicsSystem — Gravity & Velocity Integration

**Files:**
- Create: `src/engine/physics/PhysicsComponents.hpp`
- Create: `src/engine/physics/PhysicsSystem.hpp`
- Create: `src/engine/physics/PhysicsSystem.cpp`

```cpp
// PhysicsComponents.hpp
#pragma once
#include "core/core.hpp"

namespace engine::physics {
struct VelocityComponent { core::Vec2f velocity{}; };
struct GravityComponent { float gravity = 980.0f; };  // px/s²
} // namespace engine::physics
```

```cpp
// PhysicsSystem.hpp
#pragma once
#include <entt/entt.hpp>
namespace engine::physics { class PhysicsSystem; }
#include "engine/physics/PhysicsComponents.hpp"

namespace engine::physics {
class PhysicsSystem {
public:
    void FixedUpdate(entt::registry& registry, float dt);
};
} // namespace engine::physics
```

```cpp
// PhysicsSystem.cpp
#include "PhysicsSystem.hpp"
namespace engine::physics {
void PhysicsSystem::FixedUpdate(entt::registry& registry, float dt) {
    auto view = registry.view<VelocityComponent, GravityComponent>();
    for (auto entity : view) {
        auto& vel = view.get<VelocityComponent>(entity);
        float g = view.get<GravityComponent>(entity).gravity;
        vel.velocity.y += g * dt;  // apply gravity
    }
    // integrate velocity into position (TransformComponent)
    auto posView = registry.view<engine::renderer::TransformComponent, VelocityComponent>();
    for (auto entity : posView) {
        auto& pos = posView.get<engine::renderer::TransformComponent>(entity);
        auto& vel = posView.get<VelocityComponent>(entity);
        pos.position += vel.velocity * dt;
    }
}
} // namespace engine::physics
```

---
### Task 2: CollisionSystem — AABB + Ground

**Files:**
- Create: `src/engine/physics/CollisionSystem.hpp`
- Create: `src/engine/physics/CollisionSystem.cpp`
- Create: `src/engine/physics/CollisionComponents.hpp`

**Modify:** `src/engine/application/Application.cpp` — add ground and player to registry.

Implement inline.
