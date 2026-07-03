#include <catch2/catch_test_macros.hpp>
#include "math/Rect.hpp"

using namespace core;

TEST_CASE("Rect: default construction", "[core][math]") {
    Rectf r;
    CHECK(r.position == Vec2f::Zero());
    CHECK(r.size == Vec2f::Zero());
}

TEST_CASE("Rect: constructor", "[core][math]") {
    Rectf r(1.0f, 2.0f, 100.0f, 200.0f);
    CHECK(r.Left() == 1.0f);
    CHECK(r.Top() == 2.0f);
    CHECK(r.Right() == 101.0f);
    CHECK(r.Bottom() == 202.0f);
}

TEST_CASE("Rect: center", "[core][math]") {
    Rectf r(0, 0, 100, 200);
    auto c = r.Center();
    CHECK(c.x == 50.0f);
    CHECK(c.y == 100.0f);
}

TEST_CASE("Rect: contains point", "[core][math]") {
    Rectf r(10, 10, 50, 50);
    CHECK(r.Contains(Vec2f(30, 30)));
    CHECK_FALSE(r.Contains(Vec2f(5, 5)));
    CHECK(r.Contains(Vec2f(10, 10))); // inclusive edges
}

TEST_CASE("Rect: overlap", "[core][math]") {
    Rectf a(0, 0, 100, 100);
    Rectf b(50, 50, 100, 100); // overlaps a
    Rectf c(200, 0, 50, 50); // no overlap

    CHECK(a.Overlaps(b));
    CHECK(b.Overlaps(a));
    CHECK_FALSE(a.Overlaps(c));
    CHECK_FALSE(c.Overlaps(a));
}
