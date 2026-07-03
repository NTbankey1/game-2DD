#pragma once

#include "engine/scene/IScene.hpp"
#include "core/events/EventBus.hpp"
#include "game/input/InputCommand.hpp"
#include "game/input/InputMapper.hpp"
#include <vector>

namespace game {

class MenuState : public engine::IScene {
public:
    explicit MenuState(core::events::EventBus& eventBus);
    ~MenuState() override;

    void OnEnter() override;
    void OnExit() override;
    void FixedUpdate(float dt) override;
    void Update(float dt) override;
    void Render() override;

private:
    core::events::EventBus& m_eventBus;
    InputMapper m_mapper;
    std::vector<core::events::ListenerHandle> m_handles;
};

} // namespace game
