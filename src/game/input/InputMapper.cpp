#include "game/input/InputMapper.hpp"

namespace game {

InputMapper::InputMapper() { m_bindings.reserve(8); }

InputAction InputMapper::MapKey(int scancode) const {
    auto it = m_bindings.find(scancode);
    return it != m_bindings.end() ? it->second : InputAction::None;
}

void InputMapper::Bind(int scancode, InputAction action) {
    m_bindings[scancode] = action;
}

void InputMapper::Unbind(int scancode) {
    m_bindings.erase(scancode);
}

InputMapper InputMapper::CreateDefault() {
    InputMapper mapper;
    // SDL_Scancode values (stable API — used by engine layer)
    mapper.Bind(44,  InputAction::Jump);    // SDL_SCANCODE_SPACE
    mapper.Bind(82,  InputAction::Jump);    // SDL_SCANCODE_UP
    mapper.Bind(26,  InputAction::Jump);    // SDL_SCANCODE_W
    mapper.Bind(81,  InputAction::Slide);   // SDL_SCANCODE_DOWN
    mapper.Bind(22,  InputAction::Slide);   // SDL_SCANCODE_S
    mapper.Bind(41,  InputAction::Pause);   // SDL_SCANCODE_ESCAPE
    mapper.Bind(40,  InputAction::Confirm); // SDL_SCANCODE_RETURN
    return mapper;
}

} // namespace game
