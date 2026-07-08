#include "GameSaveData.hpp"
#include <nlohmann/json.hpp>

namespace game {

nlohmann::json GameSaveData::ToJson() const {
    return {
        {"score", score},
        {"highScore", highScore},
        {"coins", coins},
        {"playerHp", playerHp},
        {"playerX", playerX},
        {"playerY", playerY}
    };
}

GameSaveData GameSaveData::FromJson(const nlohmann::json& j) {
    GameSaveData d;
    if (j.contains("score")) d.score = j["score"];
    if (j.contains("highScore")) d.highScore = j["highScore"];
    if (j.contains("coins")) d.coins = j["coins"];
    if (j.contains("playerHp")) d.playerHp = j["playerHp"];
    if (j.contains("playerX")) d.playerX = j["playerX"];
    if (j.contains("playerY")) d.playerY = j["playerY"];
    return d;
}

} // namespace game
