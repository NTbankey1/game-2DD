cmake_minimum_required(VERSION 3.28)

project(EndlessRunner
    VERSION 0.1.0
    DESCRIPTION "2D Endless Runner Engine & Game"
    LANGUAGES CXX
)

# ---- C++20 required ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---- Modules ----
include(cmake/Dependencies.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/Packaging.cmake)

# ---- SDL3_ttf ----
find_package(SDL3_ttf REQUIRED)
add_compile_definitions(FONT_DIR="${CMAKE_SOURCE_DIR}/assets")

# ---- Subdirectories ----
add_subdirectory(src)
add_subdirectory(tests)
