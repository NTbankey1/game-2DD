#pragma once

#include <entt/entt.hpp>

namespace game {

/// Tag component for interactable entities
struct InteractableTag {};

/// Component storing interaction prompt text
struct InteractionComp {
    const char* prompt = "Press E";
};

/// Simple system: checks proximity between player and interactables.
/// Publishes InteractedEvent when E pressed near an interactable.
/// Call from ExplorationState::Render() after physics update.
class InteractionSystem {
public:
    /// Returns true and fills target if player is near an interactable
    bool CheckProximity(entt::registry& registry, entt::entity& outTarget);

    /// Get distance to nearest interactable (0 = none)
    float GetNearestDist(entt::registry& registry);
};

} // namespace game
