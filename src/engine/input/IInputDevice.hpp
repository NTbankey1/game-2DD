#pragma once
#include "core/core.hpp"

namespace engine {
class IInputDevice {
public:
    virtual ~IInputDevice() = default;
    virtual void PollEvents() = 0;
    virtual bool IsKeyPressed(int key) = 0;
    virtual void GetMousePos(float& x, float& y) = 0;
};
} // namespace engine
