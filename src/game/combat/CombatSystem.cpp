#include "CombatSystem.hpp"
#include "game/combat/CombatComponent.hpp"
#include "game/enemy/EnemyComponent.hpp"
#include "engine/renderer/RenderComponents.hpp"
#include "engine/physics/PhysicsComponents.hpp"

namespace game {

void CombatSystem::FixedUpdate(entt::registry& registry, core::events::EventBus& eventBus, float dt) {
    // Update invulnerability timers
    for (auto [e, hurtbox] : registry.view<HurtboxComponent>().each()) {
        if (hurtbox.invulnTimer > 0.0f) {
            hurtbox.invulnTimer -= dt;
            if (hurtbox.invulnTimer <= 0.0f) {
                hurtbox.invulnerable = false;
                hurtbox.invulnTimer = 0.0f;
            }
        }
    }

    // Check overlaps: hitbox owner AABB vs hurtbox owner AABB
    for (auto hbx : registry.view<HitboxComponent>()) {
        if (!registry.all_of<engine::renderer::TransformComponent>(hbx)) continue;
        auto& hbxPos = registry.get<engine::renderer::TransformComponent>(hbx);
        core::Vec2f hbxCenter = hbxPos.position + core::Vec2f(16, 16);

        for (auto hrx : registry.view<HurtboxComponent>()) {
            if (hrx == hbx) continue;
            auto& hurtbox = registry.get<HurtboxComponent>(hrx);
            if (hurtbox.invulnerable) continue;
            if (!registry.all_of<engine::renderer::TransformComponent>(hrx)) continue;

            auto& hrxPos = registry.get<engine::renderer::TransformComponent>(hrx);
            float dx = hbxCenter.x - hrxPos.position.x;
            float dy = hbxCenter.y - hrxPos.position.y;
            float distSq = dx * dx + dy * dy;
            if (distSq > 50.0f * 50.0f) continue; // max hit range

            auto& hitbox = registry.get<HitboxComponent>(hbx);
            hurtbox.invulnerable = true;
            hurtbox.invulnTimer = 0.3f;

            // Knockback direction
            float dir = dx > 0 ? 1.0f : -1.0f;
            core::Vec2f kb{dir * hitbox.knockbackForce, -hitbox.knockbackForce * 0.5f};

            eventBus.Publish(DamageDealtEvent{hrx, hitbox.damage, kb});

            // Check if enemy died
            if (auto* enemy = registry.try_get<EnemyComponent>(hrx)) {
                enemy->hp -= hitbox.damage;
                if (enemy->hp <= 0) {
                    eventBus.Publish(EnemyDiedEvent{hrx});
                }
            }
        }
    }
}

} // namespace game
