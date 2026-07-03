add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(game)

add_executable(endless_runner main.cpp)
target_compile_definitions(endless_runner PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(endless_runner PRIVATE
    core_core
    engine_interface
    engine_platform_sdl3
    engine_application
    game_stubs
    game_impl
)
add_compiler_warnings(endless_runner)
