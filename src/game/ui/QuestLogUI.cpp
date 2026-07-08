#include "QuestLogUI.hpp"
#include <string>

namespace game {

QuestLogUI::QuestLogUI(engine::renderer::TextRenderer& text)
    : m_text(text) {}

void QuestLogUI::Render(float x, float y, const std::vector<const QuestProgress*>& quests) {
    if (quests.empty()) {
        // Only show "no quests" briefly
        return;
    }

    m_text.RenderString("--- Quests ---", x, y, 0xAA, 0xDD, 0xAA);
    float ly = y + 14.0f;
    for (const auto* qp : quests) {
        if (!qp || qp->status != QuestStatus::Active) continue;
        m_text.RenderString(qp->questId.c_str(), x + 8.0f, ly, 0xCC, 0xFF, 0xCC);
        ly += 14.0f;
        for (const auto& obj : qp->objectives) {
            std::string line = std::to_string(obj.currentCount) + "/" + std::to_string(obj.targetCount);
            m_text.RenderString(("  " + line).c_str(), x + 16.0f, ly, 0xAA, 0xCC, 0xAA);
            ly += 14.0f;
        }
        ly += 4.0f;
    }
}

} // namespace game
