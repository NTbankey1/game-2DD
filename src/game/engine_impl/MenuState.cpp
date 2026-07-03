#include "MenuState.hpp"
#include <spdlog/spdlog.h>

namespace game {

void MenuState::OnEnter() { spdlog::info("MenuState::OnEnter"); }
void MenuState::OnExit() { spdlog::info("MenuState::OnExit"); }
void MenuState::FixedUpdate(float dt) { spdlog::trace("MenuState::FixedUpdate({:.4f})", dt); }
void MenuState::Update(float dt) { spdlog::trace("MenuState::Update({:.4f})", dt); }
void MenuState::Render() {}

} // namespace game
