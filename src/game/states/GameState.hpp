#pragma once
#include "core/core.hpp"
#include "engine/engine.hpp"

namespace game {
enum class GamePhase { Menu, Playing, Paused, GameOver };
struct GameStateComponent { GamePhase phase = GamePhase::Menu; };
} // namespace game
