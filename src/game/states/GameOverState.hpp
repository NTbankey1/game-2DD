#pragma once

#include "engine/scene/IScene.hpp"

namespace engine::application { class Application; }

namespace game {

class GameOverState : public engine::IScene {
public:
    explicit GameOverState(engine::application::Application& app);
    ~GameOverState() override;

    void OnEnter() override;
    void OnExit() override;
    void FixedUpdate(float dt) override;
    void Update(float dt) override;
    void Render() override;

private:
    engine::application::Application& m_app;
    bool m_debounceEnter = true;
};

} // namespace game
