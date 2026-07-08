#include "InventoryUI.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include <string>

namespace game {

InventoryUI::InventoryUI(core::events::EventBus& eventBus, engine::renderer::TextRenderer& text)
    : m_eventBus(eventBus), m_text(text) {
    m_handle = m_eventBus.Subscribe<InventoryChangedEvent>(
        [this](const InventoryChangedEvent& e) { OnInventoryChanged(e); });
}

InventoryUI::~InventoryUI() {
    m_eventBus.Unsubscribe(m_handle);
}

void InventoryUI::OnInventoryChanged(const InventoryChangedEvent& e) {
    // Update or add entry
    for (auto& entry : m_entries) {
        if (entry.itemId == e.itemId) {
            if (e.newQuantity <= 0) {
                // Remove
                m_entries.erase(m_entries.begin() + (&entry - m_entries.data()));
            } else {
                entry.quantity = e.newQuantity;
            }
            return;
        }
    }
    if (e.newQuantity > 0)
        m_entries.push_back({e.itemId, e.newQuantity});
}

void InventoryUI::Render(float x, float y) {
    if (m_entries.empty()) {
        m_text.RenderString("Inventory: (empty)", x, y, 0x88, 0x88, 0x88, 14);
        return;
    }
    m_text.RenderString("Inventory:", x, y, 0xAA, 0xAA, 0xCC, 14);
    float ly = y + 16.0f;
    for (const auto& entry : m_entries) {
        std::string line = "• " + entry.itemId + " x" + std::to_string(entry.quantity);
        m_text.RenderString(line.c_str(), x + 5.0f, ly, 0xCC, 0xCC, 0xFF, 14);
        ly += 16.0f;
    }
}

} // namespace game
