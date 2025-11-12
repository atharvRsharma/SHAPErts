// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include <vector>

constexpr int GRID_WIDTH = 20;
constexpr int GRID_HEIGHT = 20;

class GridSystem : public ecs::System {
public:
    void Init() {
        // Initialize grid to all false (unoccupied)
        m_Occupied.resize(GRID_WIDTH, std::vector<bool>(GRID_HEIGHT, false));
    }

    bool IsTileOccupied(int x, int y) const {
        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) {
            return true; // Out of bounds is "occupied"
        }
        return m_Occupied[x][y];
    }

    void SetTileOccupied(int x, int y, bool isOccupied) {
        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) {
            return;
        }
        m_Occupied[x][y] = isOccupied;
    }

    glm::vec3 GridToWorld(int x, int y) const {
        // The reverse of the math in InputSystem
        float posX = (float)x - (float)GRID_WIDTH / 2.0f + 0.5f;
        float posZ = (float)y - (float)GRID_HEIGHT / 2.0f + 0.5f;
        return { posX, 0.5f, posZ }; // 0.5f Y so cube sits on top
    }

    glm::ivec2 WorldToGrid(const glm::vec3& worldPos) const {
        int x = static_cast<int>(std::floor(worldPos.x + (float)GRID_WIDTH / 2.0f));
        int y = static_cast<int>(std::floor(worldPos.z + (float)GRID_HEIGHT / 2.0f));
        return { x, y };
    }

private:
    std::vector<std::vector<bool>> m_Occupied;
};