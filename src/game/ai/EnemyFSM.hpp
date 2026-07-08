#pragma once

#include <entt/entt.hpp>

namespace game {

enum class EnemyState { Patrol, Chase, Attack, Return };

/// Simple enemy AI FSM — Patrol → Chase → Attack → Return.
/// Operates on entities with EnemyComponent + PlayerTag/Transform.
class EnemyFSM {
public:
    void Update(entt::registry& registry, float dt, float playerX, float playerY);

    float m_worldLeft = 0.0f;   // patrol bounds (set from world)
    float m_worldRight = 1920.0f;
    float m_patrolRange = 100.0f; // how far from spawn point to patrol
};

} // namespace game
