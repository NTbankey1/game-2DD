#pragma once

#include "core/events/IEvent.hpp"
#include <cstdint>

namespace game {
enum class InputAction : uint8_t { Jump, Slide, Pause, Confirm, Cancel, None };

struct InputCommand : core::events::IEvent {
    InputAction action{};
    explicit InputCommand(InputAction a = InputAction::None) : action(a) {}
};
} // namespace game
