// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include <vector>
#include <glm/glm.hpp>

constexpr int GRID_WIDTH = 30;
constexpr int GRID_HEIGHT = 30;

class GridSystem : public ecs::System {
public:
    void Init() {
        // Initialize grid to all false (unoccupied)
        m_Occupied.resize(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    }

    bool IsTileOccupied(int x, int y) const {
        if (!IsValidTile(x, y)) {
            return true; // Out of bounds is "occupied"
        }
        return m_Occupied[x][y];
    }

    void SetTileOccupied(int x, int y, bool isOccupied) {
        if (IsValidTile(x, y)) {
            m_Occupied[x][y] = isOccupied;
        }
    }

    bool IsValidTile(int x, int y) const {
        return !(x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT);
    }

    //convert 2d gird coords to world space coords, on an xyz axis system
    glm::vec3 GridToWorld(int x, int y) const {
        float posX = (float)x - (float)GRID_WIDTH / 2.0f + 0.5f;
        float posZ = (float)y - (float)GRID_HEIGHT / 2.0f + 0.5f;
        return { posX, 0.0f, posZ };
    }

    // Converts world coordinates back to grid coordinates
    glm::ivec2 WorldToGrid(const glm::vec3& worldPos) const {
        int x = static_cast<int>(std::floor(worldPos.x + (float)GRID_WIDTH / 2.0f));
        int y = static_cast<int>(std::floor(worldPos.z + (float)GRID_HEIGHT / 2.0f));
        return { x, y };
    }

private:
    std::vector<std::vector<bool>> m_Occupied;
};