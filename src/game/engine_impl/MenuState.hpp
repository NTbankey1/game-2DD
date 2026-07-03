#pragma once

#include "engine/scene/IScene.hpp"

namespace game {

class MenuState : public engine::IScene {
public:
    MenuState() = default;
    ~MenuState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void FixedUpdate(float dt) override;
    void Update(float dt) override;
    void Render() override;
};

} // namespace game
