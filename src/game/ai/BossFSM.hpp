#pragma once
#include "game/enemy/EnemyComponent.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/physics/PhysicsComponents.hpp"
#include <entt/entt.hpp>
#include <cmath>

namespace game {

class BossFSM {
public:
    void Update(entt::registry& registry, float dt, float playerX, float playerY);

private:
    enum class BossAttack { Slam, Charge, Projectile };
    BossAttack m_nextAttack = BossAttack::Slam;
    float m_attackTimer = 0.0f;
    float m_phaseChangeTimer = 0.0f;
    bool m_phase2 = false;
};

} // namespace game
