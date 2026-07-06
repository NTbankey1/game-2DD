#include "EnemyFSM.hpp"
#include "game/enemy/EnemyComponent.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include <cmath>

namespace game {

void EnemyFSM::Update(entt::registry& registry, float dt, float playerX, float playerY) {
    for (auto [e, enemy, pos] : registry.view<EnemyComponent, engine::renderer::TransformComponent>().each()) {
        float dx = playerX - pos.position.x;
        float dy = playerY - pos.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        bool onSameGround = std::abs(dy) < 48.0f;

        (void)enemy;
        auto* vel = registry.try_get<engine::physics::VelocityComponent>(e);

        // Simple decision tree instead of full FSM (avoids state persistence bugs)
        if (dist < enemy.attackRange && onSameGround) {
            // Attack: stand still (attack handled by CombatSystem)
            if (vel) vel->velocity.x = 0;
        } else if (dist < enemy.visionRange && onSameGround) {
            // Chase: move toward player
            if (vel) vel->velocity.x = (dx > 0 ? 1.0f : -1.0f) * enemy.speed;
        } else if (std::abs(pos.position.x - 100.0f) > m_patrolRange) {
            // Return to spawn area
            float dir = (pos.position.x < 100.0f) ? 1.0f : -1.0f;
            if (vel) vel->velocity.x = dir * enemy.speed * 0.5f;
        } else {
            // Patrol: back and forth
            float patrolDir = std::sin(pos.position.x * 0.01f) > 0 ? 1.0f : -1.0f;
            if (vel) vel->velocity.x = patrolDir * enemy.speed * 0.3f;
        }
    }
}

} // namespace game
