#pragma once

#include <entt/entt.hpp>
#include "core/events/EventBus.hpp"

namespace game {

/// Checks hitbox-hurtbox overlaps each fixed step.
/// Publishes DamageDealtEvent, EnemyDiedEvent.
class CombatSystem {
public:
    void FixedUpdate(entt::registry& registry, core::events::EventBus& eventBus, float dt);
};

} // namespace game
