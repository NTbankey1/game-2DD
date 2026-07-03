#pragma once

#include "core/core.hpp"
#include "engine/renderer/RenderComponents.hpp"

namespace engine::physics {

struct VelocityComponent {
    core::Vec2f velocity{};
    explicit VelocityComponent(core::Vec2f v = {}) : velocity(v) {}
};

struct GravityComponent {
    float gravity = 980.0f;
    explicit GravityComponent(float g = 980.0f) : gravity(g) {}
};

struct AABBComponent {
    core::Rectf localBounds{};  // relative to entity position
    explicit AABBComponent(core::Rectf b = {}) : localBounds(b) {}
};

struct GroundTag {};    // marks static collision surface
struct PlayerTag {};    // marks player entity

} // namespace engine::physics
