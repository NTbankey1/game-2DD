#pragma once

#include "core/events/IEvent.hpp"

namespace engine::input {

struct KeyEvent : core::events::IEvent {
    int scancode{};
    bool pressed{};
    KeyEvent(int s = 0, bool p = false) : scancode(s), pressed(p) {}
};

} // namespace engine::input
