#pragma once

#include <entt/entt.hpp>
#include "core/events/IEvent.hpp"
#include <string>
#include <vector>

namespace game {

/// Component for a pickup item in the world
struct ItemComponent {
    std::string itemId;
    int quantity = 1;
};

/// For items in player's inventory (non-ECS)
struct InventoryEntry {
    std::string itemId;
    int quantity = 0;
};

// ─── Events ───

struct InventoryChangedEvent : core::events::IEvent {
    std::string itemId;
    int newQuantity = 0;
    InventoryChangedEvent() = default;
    InventoryChangedEvent(std::string id, int qty) : itemId(std::move(id)), newQuantity(qty) {}
};

struct InteractedEvent : core::events::IEvent {
    entt::entity target = entt::null;
};

} // namespace game
