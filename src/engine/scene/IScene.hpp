#pragma once
#include "core/core.hpp"

namespace engine {
class IScene {
public:
    virtual ~IScene() = default;
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void FixedUpdate(float dt) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
};
} // namespace engine
