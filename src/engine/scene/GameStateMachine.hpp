#pragma once

#include "engine/scene/SceneManager.hpp"
#include "engine/scene/IScene.hpp"
#include <memory>
#include <stack>

namespace engine::scene {

class GameStateMachine : public engine::SceneManager {
public:
    GameStateMachine() = default;
    ~GameStateMachine() override = default;

    void PushScene(std::unique_ptr<engine::IScene> scene) override;
    void PopScene() override;
    void ReloadScene() override;
    void Update(float dt) override;

    bool FixedUpdate(float dt);
    void Render();

    [[nodiscard]] size_t StateCount() const noexcept { return m_stack.size(); }
    [[nodiscard]] bool Empty() const noexcept { return m_stack.empty(); }

private:
    std::stack<std::unique_ptr<engine::IScene>> m_stack;
};

} // namespace engine::scene
