#pragma once

#include "engine/renderer/TextRenderer.hpp"
#include "game/quest/QuestData.hpp"
#include <vector>

namespace game::quest { class QuestSystem; }

namespace game {

/// Displays active quests on screen.
class QuestLogUI {
public:
    explicit QuestLogUI(engine::renderer::TextRenderer& text);

    void Render(float x, float y, const std::vector<const QuestProgress*>& quests);

private:
    engine::renderer::TextRenderer& m_text;
};

} // namespace game
