#include "ItemDatabase.hpp"

namespace game {

ItemDatabase::ItemDatabase() {
    m_items["coin"] = {"coin", "Gold Coin", "A shiny gold coin."};
    m_items["key"] = {"key", "Rusty Key", "An old rusty key. What does it open?"};
    m_items["potions"] = {"potions", "Health Potion", "Restores health."};
}

const ItemDef* ItemDatabase::GetItem(const std::string& id) const {
    auto it = m_items.find(id);
    return it != m_items.end() ? &it->second : nullptr;
}

ItemDatabase& ItemDatabase::Instance() {
    static ItemDatabase db;
    return db;
}

} // namespace game
