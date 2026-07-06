#pragma once

#include "engine/scene/IScene.hpp"
#include <memory>
#include <stack>

namespace engine::scene {

class GameStateMachine {
public:
    GameStateMachine() = default;
    ~GameStateMachine() = default;

    void PushScene(std::unique_ptr<engine::IScene> scene);
    void PopScene();
    void ReloadScene();
    void Update(float dt);
    bool FixedUpdate(float dt);
    void Render();

    [[nodiscard]] size_t StateCount() const noexcept { return m_stack.size(); }
    [[nodiscard]] bool Empty() const noexcept { return m_stack.empty(); }

private:
    std::stack<std::unique_ptr<engine::IScene>> m_stack;
};

} // namespace engine::scene
