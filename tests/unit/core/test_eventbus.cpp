#include <catch2/catch_test_macros.hpp>
#include "events/EventBus.hpp"
#include "events/IEvent.hpp"

using namespace core::events;

namespace {
    struct TestEvent : IEvent {
        int value{};
        explicit TestEvent(int v = 0) : value(v) {}
    };

    struct OtherEvent : IEvent {
        std::string msg;
        explicit OtherEvent(std::string m = {}) : msg(std::move(m)) {}
    };
}

TEST_CASE("EventBus: subscribe and publish", "[core][events]") {
    EventBus bus;
    int received = 0;

    bus.Subscribe<TestEvent>([&](const TestEvent& e) {
        received = e.value;
    });

    bus.Publish(TestEvent(42));
    REQUIRE(received == 42);
}

TEST_CASE("EventBus: multiple listeners", "[core][events]") {
    EventBus bus;
    int count = 0;

    bus.Subscribe<TestEvent>([&](const TestEvent&) { ++count; });
    bus.Subscribe<TestEvent>([&](const TestEvent&) { ++count; });

    bus.Publish(TestEvent(1));
    REQUIRE(count == 2);
}

TEST_CASE("EventBus: typed filtering", "[core][events]") {
    EventBus bus;
    int testVal = 0;
    std::string otherVal;

    bus.Subscribe<TestEvent>([&](const TestEvent& e) { testVal = e.value; });
    bus.Subscribe<OtherEvent>([&](const OtherEvent& e) { otherVal = e.msg; });

    bus.Publish(TestEvent(99));
    CHECK(testVal == 99);
    CHECK(otherVal.empty());

    bus.Publish(OtherEvent("hello"));
    CHECK(otherVal == "hello");
}

TEST_CASE("EventBus: unsubscribe", "[core][events]") {
    EventBus bus;
    int count = 0;

    auto handle = bus.Subscribe<TestEvent>([&](const TestEvent&) { ++count; });
    bus.Publish(TestEvent(1));
    REQUIRE(count == 1);

    bus.Unsubscribe(handle);
    bus.Publish(TestEvent(2));
    REQUIRE(count == 1);
}

TEST_CASE("EventBus: unsubscribe during dispatch is safe", "[core][events]") {
    EventBus bus;
    int count = 0;
    ListenerHandle handle;

    handle = bus.Subscribe<TestEvent>([&](const TestEvent&) {
        ++count;
        if (count == 1) {
            bus.Unsubscribe(handle);
        }
    });

    bus.Publish(TestEvent(1));
    REQUIRE(count == 1);
    bus.Publish(TestEvent(2));
    REQUIRE(count == 1);
}

TEST_CASE("EventBus: clear removes all listeners", "[core][events]") {
    EventBus bus;
    int count = 0;

    bus.Subscribe<TestEvent>([&](const TestEvent&) { ++count; });
    bus.Publish(TestEvent(1));
    REQUIRE(count == 1);

    bus.Clear();
    bus.Publish(TestEvent(2));
    REQUIRE(count == 1);
}

TEST_CASE("EventBus: no listeners does not crash", "[core][events]") {
    EventBus bus;
    bus.Publish(TestEvent(42));
    REQUIRE(true);
}
