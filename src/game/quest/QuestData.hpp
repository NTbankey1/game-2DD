#pragma once

#include "core/events/IEvent.hpp"
#include <string>
#include <vector>

namespace game {

enum class QuestObjectiveType { CollectItem, None };

struct QuestObjective {
    QuestObjectiveType type = QuestObjectiveType::None;
    std::string targetItemId;
    int targetCount = 0;
    int currentCount = 0;
};

struct QuestDef {
    std::string id;
    std::string name;
    std::string description;
    std::vector<QuestObjective> objectives;
};

enum class QuestStatus { NotStarted, Active, Completed };

struct QuestProgress {
    std::string questId;
    QuestStatus status = QuestStatus::NotStarted;
    int currentObjective = 0;
    std::vector<QuestObjective> objectives;
};

// ─── Events ───

struct QuestStartedEvent : core::events::IEvent {
    std::string questId;
    QuestStartedEvent() = default;
    explicit QuestStartedEvent(std::string id) : questId(std::move(id)) {}
};

struct QuestCompletedEvent : core::events::IEvent {
    std::string questId;
    QuestCompletedEvent() = default;
    explicit QuestCompletedEvent(std::string id) : questId(std::move(id)) {}
};

struct QuestProgressEvent : core::events::IEvent {
    std::string questId;
    int objectiveIndex = 0;
    int current = 0;
    int target = 0;
};

} // namespace game
