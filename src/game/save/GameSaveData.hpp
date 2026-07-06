#pragma once
#include <string>
#include <nlohmann/json_fwd.hpp>

namespace game {

struct GameSaveData {
    int score = 0;
    int highScore = 0;
    int coins = 0;
    int playerHp = 5;
    float playerX = 100.0f;
    float playerY = 500.0f;

    nlohmann::json ToJson() const;
    static GameSaveData FromJson(const nlohmann::json& j);
};

} // namespace game
