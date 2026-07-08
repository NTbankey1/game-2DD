#pragma once

#include "game/dialogue/DialogueData.hpp"
#include "core/events/EventBus.hpp"
#include <functional>

namespace game {

/// Manages active dialogue state.
/// Subscribes to InteractedEvent to start dialogue.
/// Publishes DialogueStartedEvent / DialogueEndedEvent.
class DialogueSystem {
public:
    explicit DialogueSystem(core::events::EventBus& eventBus);

    /// Start a dialogue by ID. Returns false if ID not found.
    bool StartDialogue(const std::string& id);

    /// Advance to next node. Returns false if dialogue ended.
    bool Advance();

    /// Check if dialogue is active
    [[nodiscard]] bool IsActive() const { return m_active; }

    /// Get current node (only valid when active)
    [[nodiscard]] const DialogueNode& GetCurrentNode() const { return m_nodes[m_currentIndex]; }

    /// Get total node count
    [[nodiscard]] int GetNodeCount() const { return static_cast<int>(m_nodes.size()); }

    /// Get current node index
    [[nodiscard]] int GetCurrentIndex() const { return m_currentIndex; }

    /// End dialogue early
    void EndDialogue();

    /// Register a dialogue
    void RegisterDialogue(const DialogueData& data);

    /// Set a callback for when dialogue ends
    void SetOnEnd(std::function<void()> cb) { m_onEnd = std::move(cb); }

    void Clear() { m_active = false; m_onEnd = nullptr; }

private:
    core::events::EventBus& m_eventBus;
    bool m_active = false;
    int m_currentIndex = 0;
    std::vector<DialogueNode> m_nodes;
    std::string m_currentId;
    std::vector<DialogueData> m_dialogues;
    std::function<void()> m_onEnd;
};

} // namespace game
