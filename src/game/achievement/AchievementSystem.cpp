#include "AchievementSystem.hpp"
#include <spdlog/spdlog.h>

namespace game {

AchievementSystem::AchievementSystem(core::events::EventBus& eventBus) : m_eventBus(eventBus) {
    Register({"first_steps", "First Steps", "Finish your first quest"});
    Register({"slayer", "Slayer", "Defeat 3 enemies"});
    Register({"boss_killer", "Boss Killer", "Defeat the boss"});
    Register({"collector", "Collector", "Collect 20 coins"});

    static int killCount = 0;
    m_handles.push_back(m_eventBus.Subscribe<QuestCompletedEvent>(
        [this](const QuestCompletedEvent& e) {
            if (e.questId == "coin_collector") Unlock("first_steps");
        }));
    m_handles.push_back(m_eventBus.Subscribe<EnemyDiedEvent>(
        [this](const EnemyDiedEvent&) {
            killCount++;
            if (killCount >= 3) Unlock("slayer");
        }));
}

AchievementSystem::~AchievementSystem() {
    for (auto& h : m_handles) m_eventBus.Unsubscribe(h);
    m_handles.clear();
}

void AchievementSystem::Register(const Achievement& a) {
    for (auto& ach : m_achievements)
        if (ach.id == a.id) { ach = a; return; }
    m_achievements.push_back(a);
}

void AchievementSystem::Unlock(const std::string& id) {
    auto* a = Find(id);
    if (!a || a->unlocked) return;
    a->unlocked = true;
    spdlog::info("Achievement unlocked: {} - {}", a->name, a->description);
    m_eventBus.Publish(AchievementUnlockedEvent{id, a->name});
}

bool AchievementSystem::IsUnlocked(const std::string& id) const {
    for (const auto& a : m_achievements)
        if (a.id == id) return a.unlocked;
    return false;
}

Achievement* AchievementSystem::Find(const std::string& id) {
    for (auto& a : m_achievements)
        if (a.id == id) return &a;
    return nullptr;
}

} // namespace game
