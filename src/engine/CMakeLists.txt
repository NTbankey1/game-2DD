add_library(engine_interface INTERFACE)
target_include_directories(engine_interface INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_interface INTERFACE core_core)

add_subdirectory(platform)

add_library(engine_application STATIC
    application/Application.cpp
    scene/GameStateMachine.cpp
    renderer/RenderSystem.cpp
    physics/PhysicsSystem.cpp
    physics/CollisionSystem.cpp
)
target_include_directories(engine_application PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(engine_application PUBLIC
    engine_interface
    engine_platform_sdl3
    SDL3::SDL3
)
add_compiler_warnings(engine_application)
add_sanitizers(engine_application)
