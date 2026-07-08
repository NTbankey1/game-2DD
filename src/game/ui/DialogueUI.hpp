#pragma once

#include "core/events/EventBus.hpp"
#include "engine/renderer/TextRenderer.hpp"
#include "game/dialogue/DialogueSystem.hpp"
#include "game/dialogue/DialogueEvents.hpp"

struct SDL_Renderer;

namespace game {

/// Renders dialogue box + handles advance input.
class DialogueUI {
public:
    DialogueUI(core::events::EventBus& eventBus, engine::renderer::TextRenderer& text);
    ~DialogueUI();

    void Render(SDL_Renderer* renderer, DialogueSystem& dialogueSys);

    [[nodiscard]] bool IsOpen() const { return m_open; }

private:
    core::events::EventBus& m_eventBus;
    engine::renderer::TextRenderer& m_text;
    core::events::ListenerHandle m_startHandle;
    core::events::ListenerHandle m_endHandle;
    bool m_open = false;
    std::string m_speaker;
    std::string m_textContent;

    void OnDialogueStarted(const DialogueStartedEvent& e);
    void OnDialogueEnded(const DialogueEndedEvent&);
};

} // namespace game
