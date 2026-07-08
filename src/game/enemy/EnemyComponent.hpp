#pragma once

#include <entt/entt.hpp>
#include "core/core.hpp"
#include "core/events/IEvent.hpp"

namespace game {

enum class EnemyType { Slime, Bat, Default };

struct EnemyComponent {
    EnemyType type = EnemyType::Default;
    int maxHp = 3;
    int hp = 3;
    int damage = 1;
    float speed = 60.0f;
    float attackRange = 30.0f;
    float visionRange = 200.0f;
};

struct DamageDealtEvent : core::events::IEvent {
    entt::entity target = entt::null;
    int amount = 0;
    core::Vec2f knockback{};
    DamageDealtEvent() = default;
    DamageDealtEvent(entt::entity t, int a, core::Vec2f kb) : target(t), amount(a), knockback(kb) {}
};

struct EnemyDiedEvent : core::events::IEvent {
    entt::entity entity = entt::null;
    EnemyDiedEvent() = default;
    explicit EnemyDiedEvent(entt::entity e) : entity(e) {}
};

} // namespace game
