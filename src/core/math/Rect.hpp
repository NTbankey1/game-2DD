#pragma once

#include "Vec2.hpp"

namespace core {

/// Axis-Aligned Bounding Box (Rectangle)
template<typename T>
struct Rect {
    Vec2<T> position{}; // top-left
    Vec2<T> size{}; // width, height

    constexpr Rect() noexcept = default;
    constexpr Rect(Vec2<T> pos, Vec2<T> sz) noexcept : position{pos}, size{sz} {}
    constexpr Rect(T x, T y, T w, T h) noexcept : position{x, y}, size{w, h} {}

    [[nodiscard]] constexpr T Left() const noexcept { return position.x; }
    [[nodiscard]] constexpr T Right() const noexcept { return position.x + size.x; }
    [[nodiscard]] constexpr T Top() const noexcept { return position.y; }
    [[nodiscard]] constexpr T Bottom() const noexcept { return position.y + size.y; }

    [[nodiscard]] constexpr Vec2<T> Center() const noexcept {
        return Vec2<T>{position.x + size.x / T{2}, position.y + size.y / T{2}};
    }

    [[nodiscard]] constexpr bool Contains(Vec2<T> point) const noexcept {
        return point.x >= Left() && point.x <= Right()
            && point.y >= Top() && point.y <= Bottom();
    }

    [[nodiscard]] constexpr bool Overlaps(const Rect& other) const noexcept {
        return Left() < other.Right() && Right() > other.Left()
            && Top() < other.Bottom() && Bottom() > other.Top();
    }

    constexpr bool operator==(const Rect&) const noexcept = default;
};

using Rectf = Rect<float>;
using Recti = Rect<int32_t>;

} // namespace core
