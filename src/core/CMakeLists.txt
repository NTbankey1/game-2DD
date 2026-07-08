add_library(core_core STATIC
    # No .cpp files in Phase 1 — all headers + templated
    # But we need a translation unit:
    dummy.cpp
)

target_include_directories(core_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core_core PUBLIC fmt::fmt spdlog::spdlog glm::glm EnTT::EnTT nlohmann_json::nlohmann_json cereal::cereal SDL3::SDL3)
add_compiler_warnings(core_core)
add_sanitizers(core_core)
