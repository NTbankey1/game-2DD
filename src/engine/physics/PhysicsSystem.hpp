#pragma once

#include <entt/entt.hpp>
#include "engine/physics/PhysicsComponents.hpp"

namespace engine::physics {

class PhysicsSystem {
public:
    void FixedUpdate(entt::registry& registry, float dt);
};

} // namespace engine::physics
