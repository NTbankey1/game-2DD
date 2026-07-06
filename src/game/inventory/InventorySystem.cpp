#include "InventorySystem.hpp"

namespace game {

InventorySystem::InventorySystem(core::events::EventBus& eventBus)
    : m_eventBus(eventBus) {}

void InventorySystem::AddItem(const std::string& itemId, int quantity) {
    auto* entry = Find(itemId);
    if (entry) {
        entry->quantity += quantity;
        m_eventBus.Publish(InventoryChangedEvent{itemId, entry->quantity});
        return;
    }
    m_items.push_back({itemId, quantity});
    m_eventBus.Publish(InventoryChangedEvent{itemId, quantity});
}

bool InventorySystem::RemoveItem(const std::string& itemId, int quantity) {
    auto* entry = Find(itemId);
    if (!entry || entry->quantity < quantity) return false;
    entry->quantity -= quantity;
    m_eventBus.Publish(InventoryChangedEvent{itemId, entry->quantity});
    return true;
}

int InventorySystem::GetCount(const std::string& itemId) const {
    for (const auto& e : m_items)
        if (e.itemId == itemId) return e.quantity;
    return 0;
}

InventoryEntry* InventorySystem::Find(const std::string& id) {
    for (auto& e : m_items)
        if (e.itemId == id) return &e;
    return nullptr;
}

} // namespace game
