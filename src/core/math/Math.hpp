#pragma once

#include <cmath>
#include <concepts>
#include <numbers>

namespace core {

// Arbitrary epsilon for floating comparison
template<std::floating_point T>
constexpr T Epsilon = static_cast<T>(1e-6);

// Clamp value to [min, max]
template<typename T>
constexpr T Clamp(T value, T min, T max) noexcept {
    return (value < min) ? min : (value > max) ? max : value;
}

// Linear interpolation
template<typename T>
constexpr T Lerp(T a, T b, T t) noexcept {
    return a + (b - a) * t;
}

// Degrees <-> Radians
template<std::floating_point T>
constexpr T ToRadians(T degrees) noexcept {
    return degrees * std::numbers::pi_v<T> / T{180};
}

template<std::floating_point T>
constexpr T ToDegrees(T radians) noexcept {
    return radians * T{180} / std::numbers::pi_v<T>;
}

} // namespace core
