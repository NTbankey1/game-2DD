#pragma once

#include <entt/entt.hpp>
#include "core/events/EventBus.hpp"
#include "engine/physics/PhysicsComponents.hpp"

namespace engine::physics {

struct CollisionEvent : core::events::IEvent {
    entt::entity entityA{entt::null};
    entt::entity entityB{entt::null};
    core::Vec2f overlap{};
    CollisionEvent(entt::entity a = entt::null, entt::entity b = entt::null, core::Vec2f o = {})
        : entityA(a), entityB(b), overlap(o) {}
};

class CollisionSystem {
public:
    void FixedUpdate(entt::registry& registry, core::events::EventBus& eventBus);
};

} // namespace engine::physics
