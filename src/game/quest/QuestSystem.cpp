#include "QuestSystem.hpp"
#include "game/item/ItemComponent.hpp"
#include <algorithm>

namespace game {

QuestSystem::QuestSystem(core::events::EventBus& eventBus, int initialCoins)
    : m_eventBus(eventBus) {
    // Track coin count internally from inventory events
    m_coinCount = initialCoins;
    m_invHandle = m_eventBus.Subscribe<InventoryChangedEvent>(
        [this](const InventoryChangedEvent& e) {
            if (e.itemId == "coin") m_coinCount = e.newQuantity;
            CheckQuestProgress();
        });
}

void QuestSystem::RegisterQuest(const QuestDef& quest) {
    for (auto& q : m_defs)
        if (q.id == quest.id) { q = quest; return; }
    m_defs.push_back(quest);
}

bool QuestSystem::StartQuest(const std::string& id) {
    auto* def = FindDef(id);
    if (!def) return false;

    auto* prog = FindProgress(id);
    if (prog) return false;

    QuestProgress p;
    p.questId = id;
    p.status = QuestStatus::Active;
    p.objectives = def->objectives;
    m_progress.push_back(p);
    m_eventBus.Publish(QuestStartedEvent{id});
    CheckQuestProgress();
    return true;
}

QuestProgress* QuestSystem::GetProgress(const std::string& id) {
    return FindProgress(id);
}

const QuestProgress* QuestSystem::GetProgress(const std::string& id) const {
    return const_cast<QuestSystem*>(this)->FindProgress(id);
}

std::vector<const QuestProgress*> QuestSystem::GetActiveQuests() const {
    std::vector<const QuestProgress*> result;
    for (const auto& p : m_progress)
        if (p.status == QuestStatus::Active)
            result.push_back(&p);
    return result;
}

bool QuestSystem::IsCompleted(const std::string& id) const {
    auto* p = GetProgress(id);
    return p && p->status == QuestStatus::Completed;
}

void QuestSystem::CheckQuestProgress() {
    for (auto& p : m_progress) {
        if (p.status != QuestStatus::Active) continue;

        bool allDone = true;
        for (int i = 0; i < static_cast<int>(p.objectives.size()); i++) {
            auto& obj = p.objectives[i];
            if (obj.type == QuestObjectiveType::CollectItem) {
                obj.currentCount = std::max(obj.currentCount, m_coinCount);
            }
            if (obj.currentCount < obj.targetCount)
                allDone = false;
        }

        if (allDone) {
            p.status = QuestStatus::Completed;
            m_eventBus.Publish(QuestCompletedEvent{p.questId});
        }
    }
}

QuestDef* QuestSystem::FindDef(const std::string& id) {
    for (auto& d : m_defs)
        if (d.id == id) return &d;
    return nullptr;
}

QuestProgress* QuestSystem::FindProgress(const std::string& id) {
    for (auto& p : m_progress)
        if (p.questId == id) return &p;
    return nullptr;
}

} // namespace game
