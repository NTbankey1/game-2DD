#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "math/Vec2.hpp"

using namespace core;
using Catch::Approx;

TEST_CASE("Vec2: default construction", "[core][math]") {
    Vec2f v;
    REQUIRE(v.x == 0.0f);
    REQUIRE(v.y == 0.0f);
}

TEST_CASE("Vec2: value construction", "[core][math]") {
    Vec2f v(3.0f, 4.0f);
    CHECK(v.x == 3.0f);
    CHECK(v.y == 4.0f);
}

TEST_CASE("Vec2: arithmetic", "[core][math]") {
    Vec2f a(1, 2), b(3, 4);

    SECTION("addition") {
        auto r = a + b;
        REQUIRE(r == Vec2f(4, 6));
    }
    SECTION("subtraction") {
        auto r = a - b;
        REQUIRE(r == Vec2f(-2, -2));
    }
    SECTION("scaling") {
        auto r = a * 2.0f;
        REQUIRE(r == Vec2f(2, 4));
    }
}

TEST_CASE("Vec2: length", "[core][math]") {
    Vec2f v(3, 4);
    CHECK(v.LengthSquared() == 25.0f);
    CHECK(v.Length() == 5.0f);
}

TEST_CASE("Vec2: normalization", "[core][math]") {
    Vec2f v(3, 4);
    auto n = v.Normalized();
    CHECK(n.Length() == Approx(1.0f));
    CHECK(n.x == Approx(0.6f));
    CHECK(n.y == Approx(0.8f));
}

TEST_CASE("Vec2: zero vector leaves zero", "[core][math]") {
    Vec2f z = Vec2f::Zero();
    auto n = z.Normalized();
    // Normalizing zero is a no-op (division by zero prevented)
    CHECK(n == Vec2f::Zero());
}

TEST_CASE("Vec2: dot product", "[core][math]") {
    Vec2f a(1, 0), b(0, 1);
    CHECK(Dot(a, b) == 0.0f); // perpendicular
    CHECK(Dot(a, a) == 1.0f); // unit with itself
}
