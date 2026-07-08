#include "Tilemap.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace engine::tilemap {

bool Tilemap::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Tilemap: cannot open {}", path);
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        m_width = j["width"].get<int>();
        m_height = j["height"].get<int>();
        m_tileSize = j.value("tileSize", 32);

        m_layers.clear();
        for (const auto& layerJson : j["layers"]) {
            TileLayer layer;
            layer.name = layerJson["name"].get<std::string>();
            layer.data = layerJson["data"].get<std::vector<int>>();
            if (static_cast<int>(layer.data.size()) != m_width * m_height) {
                spdlog::warn("Tilemap layer '{}' has wrong data size", layer.name);
                continue;
            }
            m_layers.push_back(std::move(layer));
        }

        spdlog::info("Tilemap loaded: {}x{} tiles, {} layers", m_width, m_height, m_layers.size());
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Tilemap parse error: {}", e.what());
        return false;
    }
}

int Tilemap::GetTile(int layer, int x, int y) const {
    if (layer < 0 || layer >= static_cast<int>(m_layers.size())) return 0;
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return 0;
    return m_layers[layer].data[y * m_width + x];
}

bool Tilemap::IsSolid(int x, int y) const {
    int tile = GetTile(1, x, y); // layer 1 = foreground/solids
    return tile >= static_cast<int>(TileID::Ground);
}

bool Tilemap::IsSolidWorld(float worldX, float worldY) const {
    int tx = static_cast<int>(worldX) / m_tileSize;
    int ty = static_cast<int>(worldY) / m_tileSize;
    return IsSolid(tx, ty);
}

} // namespace engine::tilemap
