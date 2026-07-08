#pragma once

#include "game/quest/QuestData.hpp"
#include "game/item/ItemComponent.hpp"
#include "core/events/EventBus.hpp"
#include <vector>

namespace game::inventory { class InventorySystem; }

namespace game {

/// Manages quest lifecycle — start, track progress, complete.
/// Subscribes to InventoryChangedEvent to auto-check item collection goals.
class QuestSystem {
public:
    QuestSystem(core::events::EventBus& eventBus, int initialCoins = 0);

    /// Register a quest definition
    void RegisterQuest(const QuestDef& quest);

    /// Start a quest by ID
    bool StartQuest(const std::string& id);

    /// Get progress for a quest
    QuestProgress* GetProgress(const std::string& id);
    const QuestProgress* GetProgress(const std::string& id) const;

    /// Get all active quests
    std::vector<const QuestProgress*> GetActiveQuests() const;

    /// Check if a quest is completed
    bool IsCompleted(const std::string& id) const;

    /// For manual check (called when inventory changes)
    void CheckQuestProgress();

    void Clear() { m_progress.clear(); }

private:
    core::events::EventBus& m_eventBus;
    std::vector<QuestDef> m_defs;
    std::vector<QuestProgress> m_progress;
    core::events::ListenerHandle m_invHandle;
    int m_coinCount = 0;

    QuestDef* FindDef(const std::string& id);
    QuestProgress* FindProgress(const std::string& id);
};

} // namespace game
