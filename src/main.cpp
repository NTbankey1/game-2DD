#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "core/core.hpp"

int main(int argc, char* argv[]) {
    // Init logging
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    spdlog::info("Endless Runner v{}", PROJECT_VERSION);
    spdlog::info("C++ standard: {}", __cplusplus);
    spdlog::info("Core library loaded — Phase 1 ready");

    // Quick smoke test: Vec2
    core::Vec2f v(3, 4);
    spdlog::info("Vec2({:.1f}, {:.1f}).Length() = {:.1f}", v.x, v.y, v.Length());

    spdlog::info("Endless Runner initialized successfully");
    spdlog::info("Build: {} @ {}", __DATE__, __TIME__);

    return 0;
}
