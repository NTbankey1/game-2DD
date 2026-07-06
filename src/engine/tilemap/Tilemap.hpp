#pragma once

#include "core/core.hpp"
#include <vector>
#include <string>

namespace engine::tilemap {

enum class TileID : int {
    Empty = 0,
    Ground = 1,
    GrassTop = 2,
    Platform = 3,
    Rock = 4,
};

struct TileLayer {
    std::string name;
    std::vector<int> data; // row-major, width * height
};

class Tilemap {
public:
    Tilemap() = default;

    bool LoadFromFile(const std::string& path);

    [[nodiscard]] int GetWidth() const noexcept { return m_width; }
    [[nodiscard]] int GetHeight() const noexcept { return m_height; }
    [[nodiscard]] int GetTileSize() const noexcept { return m_tileSize; }
    [[nodiscard]] float GetWorldWidth() const noexcept { return static_cast<float>(m_width * m_tileSize); }
    [[nodiscard]] float GetWorldHeight() const noexcept { return static_cast<float>(m_height * m_tileSize); }
    [[nodiscard]] core::Vec2f GetWorldSize() const noexcept {
        return core::Vec2f(GetWorldWidth(), GetWorldHeight());
    }

    [[nodiscard]] int GetTile(int layer, int x, int y) const;
    [[nodiscard]] bool IsSolid(int x, int y) const;
    [[nodiscard]] bool IsSolidWorld(float worldX, float worldY) const;
    [[nodiscard]] int LayerCount() const noexcept { return static_cast<int>(m_layers.size()); }

    const TileLayer& GetLayer(int idx) const { return m_layers[idx]; }

private:
    int m_width = 0;
    int m_height = 0;
    int m_tileSize = 32;
    std::vector<TileLayer> m_layers;
};

} // namespace engine::tilemap
