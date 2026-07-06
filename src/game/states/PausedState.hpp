#pragma once

#include "engine/scene/IScene.hpp"

namespace engine::application { class Application; }

namespace game {

class PausedState : public engine::IScene {
public:
    explicit PausedState(engine::application::Application& app);
    ~PausedState() override;

    void OnEnter() override;
    void OnExit() override;
    void FixedUpdate(float dt) override;
    void Update(float dt) override;
    void Render() override;

private:
    engine::application::Application& m_app;
};

} // namespace game
