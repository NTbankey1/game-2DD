#include <catch2/catch_test_macros.hpp>
#include "engine/camera/Camera.hpp"

using namespace core;
using namespace engine;

TEST_CASE("Camera: default state", "[engine][camera]") {
    Camera cam;
    CHECK(cam.GetPosition() == Vec2f(0, 0));
    CHECK(cam.GetZoom() == 1.0f);
    CHECK(cam.GetViewport() == Vec2f(1280, 720));
}

TEST_CASE("Camera: set position", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(100, 200));
    CHECK(cam.GetPosition() == Vec2f(100, 200));
}

TEST_CASE("Camera: move", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(100, 100));
    cam.Move(Vec2f(10, -20));
    CHECK(cam.GetPosition() == Vec2f(110, 80));
}

TEST_CASE("Camera: zoom", "[engine][camera]") {
    Camera cam;
    cam.SetZoom(2.0f);
    CHECK(cam.GetZoom() == 2.0f);
}

TEST_CASE("Camera: bounds", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(100, 50));
    cam.SetViewport(Vec2f(800, 600));
    auto b = cam.GetBounds();
    CHECK(b.position == Vec2f(100, 50));
    CHECK(b.size == Vec2f(800, 600));
}

TEST_CASE("Camera: parallax offset", "[engine][camera]") {
    Camera cam;
    cam.SetPosition(Vec2f(500, 300));
    auto fg = cam.GetParallaxOffset(1.0f);
    CHECK(fg == Vec2f(500, 300));
    auto bg = cam.GetParallaxOffset(0.25f);
    CHECK(bg == Vec2f(125, 75));
}
