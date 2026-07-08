#pragma once

#include "core/events/IEvent.hpp"
#include "core/events/EventBus.hpp"
#include "game/quest/QuestData.hpp"
#include "game/enemy/EnemyComponent.hpp"
#include <string>
#include <vector>

namespace game {

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    bool unlocked = false;
};

struct AchievementUnlockedEvent : core::events::IEvent {
    std::string id;
    std::string name;
    AchievementUnlockedEvent() = default;
    AchievementUnlockedEvent(std::string i, std::string n) : id(std::move(i)), name(std::move(n)) {}
};

class AchievementSystem {
public:
    explicit AchievementSystem(core::events::EventBus& eventBus);
    ~AchievementSystem();

    void Register(const Achievement& a);
    void Unlock(const std::string& id);
    bool IsUnlocked(const std::string& id) const;
    const std::vector<Achievement>& GetAll() const { return m_achievements; }
    void Clear() { m_achievements.clear(); }

private:
    core::events::EventBus& m_eventBus;
    std::vector<Achievement> m_achievements;
    std::vector<core::events::ListenerHandle> m_handles;

    Achievement* Find(const std::string& id);
};

} // namespace game
