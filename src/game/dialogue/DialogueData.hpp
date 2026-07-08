#pragma once

#include <string>
#include <vector>

namespace game {

/// A dialogue node — linear chat, no branching (for now).
struct DialogueNode {
    std::string speaker;
    std::string text;
};

/// Dialogue data: an ordered list of nodes + a unique ID.
struct DialogueData {
    std::string id;
    std::vector<DialogueNode> nodes;
};

} // namespace game
