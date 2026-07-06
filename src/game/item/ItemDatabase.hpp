#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace game {

struct ItemDef {
    std::string id;
    std::string name;
    std::string description;
};

/// Simple item database — holds definitions loaded inline for now.
class ItemDatabase {
public:
    ItemDatabase();

    const ItemDef* GetItem(const std::string& id) const;
    const std::unordered_map<std::string, ItemDef>& AllItems() const { return m_items; }

    static ItemDatabase& Instance();

private:
    std::unordered_map<std::string, ItemDef> m_items;
};

} // namespace game
