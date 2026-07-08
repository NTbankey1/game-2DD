#pragma once

#include <string>
#include <vector>
#include "core/events/EventBus.hpp"
#include "game/item/ItemComponent.hpp"

namespace game {

/// Simple inventory — stores item counts, publishes events on change.
class InventorySystem {
public:
    explicit InventorySystem(core::events::EventBus& eventBus);

    void AddItem(const std::string& itemId, int quantity = 1);
    bool RemoveItem(const std::string& itemId, int quantity = 1);
    int GetCount(const std::string& itemId) const;

    const std::vector<InventoryEntry>& GetAll() const { return m_items; }
    void Clear() { m_items.clear(); }

private:
    core::events::EventBus& m_eventBus;
    std::vector<InventoryEntry> m_items;

    InventoryEntry* Find(const std::string& id);
};

} // namespace game
