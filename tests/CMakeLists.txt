add_executable(unit_tests
    main.cpp
    unit/core/test_vec2.cpp
    unit/core/test_rect.cpp
    unit/core/test_eventbus.cpp
    unit/engine/test_layer_stubs.cpp
)

target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain core_core)
add_compiler_warnings(unit_tests)
add_sanitizers(unit_tests)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
