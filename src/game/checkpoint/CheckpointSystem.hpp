#pragma once

#include "core/core.hpp"
#include "core/events/IEvent.hpp"
#include <entt/entt.hpp>

namespace game {

struct CheckpointComponent {
    core::Vec2f respawnPos{};
    bool activated = false;
};

struct CheckpointReachedEvent : core::events::IEvent {
    core::Vec2f pos{};
};

class CheckpointSystem {
public:
    void CheckActivation(entt::registry& registry, float playerX, float playerY);
    core::Vec2f GetRespawnPos(const entt::registry& registry) const;
    void SetDefaultRespawn(core::Vec2f pos) { m_defaultRespawn = pos; }
private:
    core::Vec2f m_defaultRespawn{100.0f, 500.0f};
};

} // namespace game
