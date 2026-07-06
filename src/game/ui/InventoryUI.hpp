#pragma once

#include "core/events/EventBus.hpp"
#include "game/item/ItemComponent.hpp"
#include <vector>

namespace engine::renderer { class TextRenderer; }

namespace game {

/// Displays inventory items on the HUD and listens for changes.
class InventoryUI {
public:
    InventoryUI(core::events::EventBus& eventBus, engine::renderer::TextRenderer& text);
    ~InventoryUI();

    void Render(float x, float y);
    void Clear() { m_entries.clear(); }

private:
    core::events::EventBus& m_eventBus;
    engine::renderer::TextRenderer& m_text;
    core::events::ListenerHandle m_handle;
    std::vector<InventoryEntry> m_entries;

    void OnInventoryChanged(const InventoryChangedEvent& e);
};

} // namespace game
