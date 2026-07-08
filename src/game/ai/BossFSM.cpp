#include "BossFSM.hpp"
#include "game/combat/CombatComponent.hpp"

namespace game {

void BossFSM::Update(entt::registry& registry, float dt, float playerX, float playerY) {
    for (auto [e, enemy, pos] : registry.view<EnemyComponent, engine::renderer::TransformComponent>().each()) {
        if (enemy.type != EnemyType::Default) continue; // skip non-boss
        if (enemy.maxHp < 10) continue; // not a boss

        float dx = playerX - pos.position.x;
        float dy = playerY - pos.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        auto* vel = registry.try_get<engine::physics::VelocityComponent>(e);

        // Phase 2 at 50% HP
        if (enemy.hp <= enemy.maxHp / 2 && !m_phase2) {
            m_phase2 = true;
            m_attackTimer = 0.0f;
        }

        m_attackTimer += dt;
        float phaseMultiplier = m_phase2 ? 0.7f : 1.2f;

        if (dist < 60.0f) {
            // Slam attack
            if (vel) vel->velocity.x = dx > 0 ? -120.0f : 120.0f;
            if (vel) vel->velocity.y = -300.0f;
            m_attackTimer = 0.0f;
            m_nextAttack = BossAttack::Charge;
        } else if (dist < 250.0f && m_attackTimer > phaseMultiplier) {
            // Charge toward player
            float dir = dx > 0 ? 1.0f : -1.0f;
            if (vel) vel->velocity.x = dir * (m_phase2 ? 250.0f : 200.0f);
            if (dist > 100.0f && vel) vel->velocity.y = -100.0f; // hop
            m_attackTimer = 0.0f;
            m_nextAttack = BossAttack::Slam;
        } else {
            // Approach player slowly
            float dir = dx > 0 ? 1.0f : -1.0f;
            if (vel) vel->velocity.x = dir * 40.0f;
        }
    }
}

} // namespace game
