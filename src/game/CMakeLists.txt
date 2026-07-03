add_library(game_stubs INTERFACE)
target_include_directories(game_stubs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game_stubs INTERFACE engine_interface)

add_library(game_impl STATIC
    engine_impl/MenuState.cpp
    input/InputMapper.cpp
)
target_include_directories(game_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game_impl PUBLIC engine_application engine_interface)
add_compiler_warnings(game_impl)
add_sanitizers(game_impl)
