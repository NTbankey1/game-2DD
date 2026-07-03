#include "engine/application/Application.hpp"
#include "engine/scene/GameStateMachine.hpp"
#include "game/engine_impl/MenuState.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");

    spdlog::info("Endless Runner v{}", PROJECT_VERSION);

    engine::application::Application app;
    if (!app.Initialize()) {
        spdlog::error("Application initialization failed");
        return 1;
    }

    app.States().PushScene(std::make_unique<game::MenuState>(app.Events()));
    app.Run();

    spdlog::info("Endless Runner exited cleanly");
    return 0;
}
