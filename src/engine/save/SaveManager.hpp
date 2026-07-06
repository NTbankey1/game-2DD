#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace engine::save {

class SaveManager {
public:
    static bool SaveToFile(const std::string& path, const nlohmann::json& data);
    static nlohmann::json LoadFromFile(const std::string& path);
    static bool FileExists(const std::string& path);
};

} // namespace engine::save
