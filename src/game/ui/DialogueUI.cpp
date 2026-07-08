#include "DialogueUI.hpp"
#include "game/dialogue/DialogueEvents.hpp"
#include <SDL3/SDL.h>

namespace game {

DialogueUI::DialogueUI(core::events::EventBus& eventBus, engine::renderer::TextRenderer& text)
    : m_eventBus(eventBus), m_text(text) {
    m_startHandle = m_eventBus.Subscribe<DialogueStartedEvent>(
        [this](const DialogueStartedEvent& e) { OnDialogueStarted(e); });
    m_endHandle = m_eventBus.Subscribe<DialogueEndedEvent>(
        [this](const DialogueEndedEvent& e) { OnDialogueEnded(e); });
}

DialogueUI::~DialogueUI() {
    m_eventBus.Unsubscribe(m_startHandle);
    m_eventBus.Unsubscribe(m_endHandle);
}

void DialogueUI::OnDialogueStarted(const DialogueStartedEvent& /*e*/) {
    m_open = true;
}

void DialogueUI::OnDialogueEnded(const DialogueEndedEvent&) {
    m_open = false;
}

void DialogueUI::Render(SDL_Renderer* renderer, DialogueSystem& dialogueSys) {
    if (!m_open) return;

    // Dialogue box background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x0A, 0x0A, 0x1A, 220);
    SDL_FRect bg{80, 510, 1120, 170};
    SDL_RenderFillRect(renderer, &bg);
    // Border
    SDL_SetRenderDrawColor(renderer, 0x44, 0x66, 0xAA, 200);
    SDL_FRect border{80, 510, 1120, 2};
    SDL_RenderFillRect(renderer, &border);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Speaker name
    const auto& node = dialogueSys.GetCurrentNode();
    std::string spk = node.speaker + ":";
    m_text.RenderString(spk.c_str(), 110, 525, 0xFF, 0xCC, 0x44, 18);

    // Dialogue text
    m_text.RenderString(node.text.c_str(), 110, 550, 0xEE, 0xEE, 0xFF, 16);

    // Continue prompt
    int nodeIdx = dialogueSys.GetCurrentIndex();
    int nodeCnt = dialogueSys.GetNodeCount();
    if (nodeIdx < nodeCnt - 1) {
        m_text.RenderString("[E] continue", 600, 640, 0xAA, 0xAA, 0x88, 14);
    } else {
        m_text.RenderString("[E] close", 610, 640, 0x88, 0xCC, 0x88, 14);
    }
}

} // namespace game
