#pragma once

#include <string>

namespace game {

/// Component for NPC entities.
struct NPCComponent {
    std::string name = "Stranger";
    std::string dialogueId = "test_greeting";
};

} // namespace game
