#pragma once

#include "game/input/InputCommand.hpp"
#include <unordered_map>

namespace game {

class InputMapper {
public:
    InputMapper();

    InputAction MapKey(int scancode) const;
    void Bind(int scancode, InputAction action);
    void Unbind(int scancode);

    static InputMapper CreateDefault();

    [[nodiscard]] const auto& Bindings() const { return m_bindings; }

private:
    std::unordered_map<int, InputAction> m_bindings;
};

} // namespace game
