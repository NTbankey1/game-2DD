#include "DialogueSystem.hpp"
#include "DialogueEvents.hpp"

namespace game {

DialogueSystem::DialogueSystem(core::events::EventBus& eventBus)
    : m_eventBus(eventBus) {}

void DialogueSystem::RegisterDialogue(const DialogueData& data) {
    for (auto& d : m_dialogues) {
        if (d.id == data.id) { d = data; return; }
    }
    m_dialogues.push_back(data);
}

bool DialogueSystem::StartDialogue(const std::string& id) {
    for (const auto& d : m_dialogues) {
        if (d.id == id) {
            m_nodes = d.nodes;
            m_currentId = id;
            m_currentIndex = 0;
            m_active = true;
            m_eventBus.Publish(DialogueStartedEvent(id));
            return true;
        }
    }
    return false;
}

bool DialogueSystem::Advance() {
    if (!m_active) return false;
    m_currentIndex++;
    if (m_currentIndex >= static_cast<int>(m_nodes.size())) {
        EndDialogue();
        return false;
    }
    return true;
}

void DialogueSystem::EndDialogue() {
    if (!m_active) return;
    m_active = false;
    m_eventBus.Publish(DialogueEndedEvent());
    if (m_onEnd) m_onEnd();
}

} // namespace game
