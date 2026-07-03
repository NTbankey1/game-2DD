#pragma once
#include "core/core.hpp"

namespace game {
enum class ObstacleType { Spike, Bird, Barrier };
struct ObstacleTag { ObstacleType type{}; };
} // namespace game
