add_library(engine_platform_sdl3 STATIC
    sdl3/SDLWindow.cpp
    sdl3/SDLInputDevice.cpp
)
target_include_directories(engine_platform_sdl3 PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/sdl3
)
target_link_libraries(engine_platform_sdl3 PUBLIC engine_interface SDL3::SDL3)
add_compiler_warnings(engine_platform_sdl3)
add_sanitizers(engine_platform_sdl3)
