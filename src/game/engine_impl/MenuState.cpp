#include "MenuState.hpp"
#include "engine/input/KeyEvent.hpp"
#include "game/input/InputCommand.hpp"
#include <spdlog/spdlog.h>

namespace game {

MenuState::MenuState(core::events::EventBus& eventBus)
    : m_eventBus(eventBus)
    , m_mapper(InputMapper::CreateDefault())
{
    m_handles.reserve(4);
}

MenuState::~MenuState() {
    for (auto& h : m_handles) m_eventBus.Unsubscribe(h);
    m_handles.clear();
}

void MenuState::OnEnter() {
    spdlog::info("MenuState::OnEnter");

    // Subscribe to key events → convert to InputCommands
    m_handles.push_back(m_eventBus.Subscribe<engine::input::KeyEvent>([this](const engine::input::KeyEvent& e) {
        if (!e.pressed) return;  // only react on key down
        InputAction action = m_mapper.MapKey(e.scancode);
        if (action != InputAction::None) {
            m_eventBus.Publish(InputCommand{action});
        }
    }));

    // Subscribe to our own InputCommands for debug logging
    m_handles.push_back(m_eventBus.Subscribe<InputCommand>([this](const InputCommand& cmd) {
        switch (cmd.action) {
        case InputAction::Jump:    spdlog::info("Input: Jump");    break;
        case InputAction::Slide:   spdlog::info("Input: Slide");   break;
        case InputAction::Pause:   spdlog::info("Input: Pause");   break;
        case InputAction::Confirm: spdlog::info("Input: Confirm"); break;
        case InputAction::Cancel:  spdlog::info("Input: Cancel");  break;
        default: break;
        }
    }));
}

void MenuState::OnExit() {
    spdlog::info("MenuState::OnExit");
    for (auto& h : m_handles) m_eventBus.Unsubscribe(h);
    m_handles.clear();
}

void MenuState::FixedUpdate(float dt) { spdlog::trace("MenuState::FixedUpdate({:.4f})", dt); }
void MenuState::Update(float dt) { spdlog::trace("MenuState::Update({:.4f})", dt); }
void MenuState::Render() {}

} // namespace game
