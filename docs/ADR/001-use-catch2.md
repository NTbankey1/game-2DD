# ADR-001: Use Catch2 for Unit Testing

**Context:** Need a C++20-compatible test framework.

**Decision:** Use Catch2 v3 (header-only, single header or compiled).

**Reasoning:**
- Most mature C++20 test framework
- Built-in CTest integration (`catch_discover_tests`)
- BDD-style sections enable shared fixture setup
- Active maintenance, CMake FetchContent compatible

**Tradeoffs:**
- Slightly slower compilation vs doctest (acceptable at this scale)
- Richer assertion macros than GoogleTest (no REQUIRE_THAT complexity)

**Revisit when:** Test suite exceeds 1000 cases and compile time is measurable.
