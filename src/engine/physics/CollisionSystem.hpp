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

struct ObstacleTag {};

struct PlayerDiedEvent : core::events::IEvent {
    int finalScore = 0;
    explicit PlayerDiedEvent(int s = 0) : finalScore(s) {}
};

class CollisionSystem {
public:
    void FixedUpdate(entt::registry& registry, core::events::EventBus& eventBus, bool& playerDead, int& score);
};

} // namespace engine::physics
