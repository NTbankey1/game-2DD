#pragma once

#include <cmath>

namespace core {

/// 2D vector with X, Y components
template<typename T>
struct Vec2 {
    T x{};
    T y{};

    constexpr Vec2() noexcept = default;
    constexpr Vec2(T x_, T y_) noexcept : x{x_}, y{y_} {}

    // Unary ops
    constexpr Vec2 operator+() const noexcept { return *this; }
    constexpr Vec2 operator-() const noexcept { return Vec2{-x, -y}; }

    // Arithmetic
    constexpr Vec2 operator+(Vec2 rhs) const noexcept { return Vec2{x + rhs.x, y + rhs.y}; }
    constexpr Vec2 operator-(Vec2 rhs) const noexcept { return Vec2{x - rhs.x, y - rhs.y}; }
    constexpr Vec2 operator*(T s) const noexcept { return Vec2{x * s, y * s}; }
    constexpr Vec2 operator/(T s) const noexcept { return Vec2{x / s, y / s}; }

    constexpr Vec2& operator+=(Vec2 rhs) noexcept { x += rhs.x; y += rhs.y; return *this; }
    constexpr Vec2& operator-=(Vec2 rhs) noexcept { x -= rhs.x; y -= rhs.y; return *this; }
    constexpr Vec2& operator*=(T s) noexcept { x *= s; y *= s; return *this; }
    constexpr Vec2& operator/=(T s) noexcept { x /= s; y /= s; return *this; }

    // Comparison
    constexpr bool operator==(const Vec2& rhs) const noexcept = default;

    // Magnitude
    [[nodiscard]] T LengthSquared() const noexcept { return x * x + y * y; }
    [[nodiscard]] T Length() const { return static_cast<T>(std::sqrt(LengthSquared())); }

    // Normalize in-place
    Vec2& Normalize() {
        T len = Length();
        if (len > T{0}) { *this /= len; }
        return *this;
    }

    [[nodiscard]] Vec2 Normalized() const {
        Vec2 r = *this;
        r.Normalize();
        return r;
    }

    // Static helpers
    static constexpr Vec2 Zero() noexcept { return Vec2{T{0}, T{0}}; }
    static constexpr Vec2 One() noexcept { return Vec2{T{1}, T{1}}; }
    static constexpr Vec2 Up() noexcept { return Vec2{T{0}, T{-1}}; }
    static constexpr Vec2 Down() noexcept { return Vec2{T{0}, T{1}}; }
    static constexpr Vec2 Left() noexcept { return Vec2{T{-1}, T{0}}; }
    static constexpr Vec2 Right() noexcept { return Vec2{T{1}, T{0}}; }
};

// Scalar * Vec2
template<typename T>
constexpr Vec2<T> operator*(T s, Vec2<T> v) noexcept {
    return Vec2<T>{v.x * s, v.y * s};
}

// Dot product
template<typename T>
constexpr T Dot(Vec2<T> a, Vec2<T> b) noexcept {
    return a.x * b.x + a.y * b.y;
}

// Distance
template<typename T>
T Distance(Vec2<T> a, Vec2<T> b) noexcept {
    return (a - b).Length();
}

// Common aliases
using Vec2f = Vec2<float>;
using Vec2i = Vec2<int32_t>;
using Vec2u = Vec2<uint32_t>;

} // namespace core
