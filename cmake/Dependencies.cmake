# All third-party dependencies via FetchContent
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# ---- fmt (string formatting) ----
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(fmt)

# ---- spdlog (logging) ----
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.1
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

# ---- GLM (math) ----
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
set(GLMI_TEST_ENABLE OFF CACHE BOOL "" FORCE)
set(GLMI_INSTALL_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# ---- EnTT (ECS) ----
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.14.0
)
set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENTT_BUILD_LIB OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(EnTT)

# ---- nlohmann/json ----
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# ---- cereal (serialization) ----
FetchContent_Declare(
    cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal.git
    GIT_TAG v1.3.2
)
set(CEREAL_INSTALL OFF CACHE BOOL "" FORCE)
set(JUST_INSTALL_CEREAL OFF CACHE BOOL "" FORCE)
set(BUILD_SANDBOX OFF CACHE BOOL "" FORCE)
set(WITH_WERROR OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cereal)

# ---- SDL3 (window, input, rendering) ----
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
set(SDL3_EXAMPLES_ENABLED OFF CACHE BOOL "" FORCE)
set(SDL3_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
set(SDL3_INSTALL_ENABLED OFF CACHE BOOL "" FORCE)
set(SDL_X11 OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

# ---- Catch2 (testing) ----
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Catch2)
