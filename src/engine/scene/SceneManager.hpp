#pragma once
#include "core/core.hpp"
#include "IScene.hpp"
#include <memory>
#include <stack>

namespace engine {
class SceneManager {
public:
    virtual ~SceneManager() = default;
    virtual void PushScene(std::unique_ptr<IScene> scene) = 0;
    virtual void PopScene() = 0;
    virtual void ReloadScene() = 0;
    virtual void Update(float dt) = 0;
};
} // namespace engine
