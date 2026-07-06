#include "SaveManager.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

namespace engine::save {

bool SaveManager::SaveToFile(const std::string& path, const nlohmann::json& data) {
    std::ofstream file(path);
    if (!file.is_open()) {
        spdlog::error("SaveManager: cannot write {}", path);
        return false;
    }
    file << data.dump(2);
    spdlog::info("SaveManager: saved to {}", path);
    return true;
}

nlohmann::json SaveManager::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::warn("SaveManager: cannot read {}", path);
        return nlohmann::json{};
    }
    try {
        nlohmann::json j;
        file >> j;
        spdlog::info("SaveManager: loaded from {}", path);
        return j;
    } catch (...) {
        spdlog::error("SaveManager: corrupted save file {}", path);
        return nlohmann::json{};
    }
}

bool SaveManager::FileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

} // namespace engine::save
