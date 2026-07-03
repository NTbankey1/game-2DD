#include "GameStateMachine.hpp"
#include <spdlog/spdlog.h>

namespace engine::scene {

void GameStateMachine::PushScene(std::unique_ptr<engine::IScene> scene) {
    if (scene) {
        scene->OnEnter();
        m_stack.push(std::move(scene));
        spdlog::info("GameStateMachine: pushed state (depth={})", m_stack.size());
    }
}

void GameStateMachine::PopScene() {
    if (m_stack.empty()) return;
    m_stack.top()->OnExit();
    m_stack.pop();
    spdlog::info("GameStateMachine: popped state (depth={})", m_stack.size());
    if (!m_stack.empty()) {
        m_stack.top()->OnEnter();
    }
}

void GameStateMachine::ReloadScene() {
    if (m_stack.empty()) return;
    m_stack.top()->OnExit();
    m_stack.pop();
    spdlog::info("GameStateMachine: scene popped for reload");
}

void GameStateMachine::Update(float dt) {
    if (m_stack.empty()) return;
    m_stack.top()->Update(dt);
}

bool GameStateMachine::FixedUpdate(float dt) {
    if (m_stack.empty()) return false;
    m_stack.top()->FixedUpdate(dt);
    return true;
}

void GameStateMachine::Render() {
    if (m_stack.empty()) return;
    m_stack.top()->Render();
}

} // namespace engine::scene
