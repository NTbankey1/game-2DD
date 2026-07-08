#pragma once

#include "core/events/IEvent.hpp"
#include <string>

namespace game {

/// Published when dialogue starts
struct DialogueStartedEvent : core::events::IEvent {
    std::string dialogueId;
    explicit DialogueStartedEvent(std::string id) : dialogueId(std::move(id)) {}
};

/// Published when dialogue ends
struct DialogueEndedEvent : core::events::IEvent {};

} // namespace game
