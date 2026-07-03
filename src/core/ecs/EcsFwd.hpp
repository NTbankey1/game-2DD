#pragma once

#include <entt/entt.hpp>

namespace core::ecs {

// Type aliases — thin, no overhead
using Entity = entt::entity;
using Registry = entt::registry;

// Sentinel for "no entity"
constexpr Entity NullEntity = entt::null;

} // namespace core::ecs
