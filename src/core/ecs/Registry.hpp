#pragma once

#include "EcsFwd.hpp"

namespace core::ecs {

// Helper functions (free functions, not methods on Registry — avoids wrapping EnTT)

/// Create an entity with no components
[[nodiscard]] inline Entity CreateEntity(Registry& reg) {
    return reg.create();
}

/// Destroy entity and all its components
inline void DestroyEntity(Registry& reg, Entity e) {
    reg.destroy(e);
}

/// Emplace component (construct in-place)
template<typename T, typename... Args>
T& EmplaceComponent(Registry& reg, Entity e, Args&&... args) {
    return reg.emplace<T>(e, std::forward<Args>(args)...);
}

/// Get component (asserts exists)
template<typename T>
T& GetComponent(Registry& reg, Entity e) {
    return reg.get<T>(e);
}

/// Get component, const
template<typename T>
const T& GetComponent(const Registry& reg, Entity e) {
    return reg.get<T>(e);
}

/// Check if entity has component
template<typename T>
bool HasComponent(const Registry& reg, Entity e) {
    return reg.all_of<T>(e);
}

/// Remove component
template<typename T>
void RemoveComponent(Registry& reg, Entity e) {
    reg.remove<T>(e);
}

/// View all entities with given components
template<typename... Components>
auto View(Registry& reg) {
    return reg.view<Components...>();
}

/// View (const)
template<typename... Components>
auto View(const Registry& reg) {
    return reg.view<Components...>();
}

} // namespace core::ecs
