#pragma once

#include "ECS.h" 
#include <vector>
#include <glm/glm.hpp>

constexpr int GRID_WIDTH = 20;
constexpr int GRID_HEIGHT = 20;

class GridSystem : public ecs::System {
public:
    void Init() {
        m_Grid.resize(GRID_WIDTH, std::vector<ecs::Entity>(GRID_HEIGHT, ecs::MAX_ENTITIES));
    }

    bool IsTileOccupied(int x, int y) const {
        if (!IsValidTile(x, y)) {
            return true;
        }
        return m_Grid[x][y] != ecs::MAX_ENTITIES;
    }

    ecs::Entity GetEntityAt(int x, int y) const {
        if (!IsValidTile(x, y)) {
            return ecs::MAX_ENTITIES;
        }
        return m_Grid[x][y];
    }

    void SetEntityAt(int x, int y, ecs::Entity entity) {
        if (IsValidTile(x, y)) {
            m_Grid[x][y] = entity;
        }
    }

    bool IsWalkable(int x, int y) const {
        return IsValidTile(x, y) && !IsTileOccupied(x, y);
    }

    bool IsValidTile(int x, int y) const {
        return !(x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT);
    }

    glm::vec3 GridToWorld(int x, int y) const {
        float posX = (float)x - (float)GRID_WIDTH / 2.0f + 0.5f;
        float posZ = (float)y - (float)GRID_HEIGHT / 2.0f + 0.5f;
        return { posX, 0.0f, posZ };
    }

    glm::ivec2 WorldToGrid(const glm::vec3& worldPos) const {
        int x = static_cast<int>(std::floor(worldPos.x + (float)GRID_WIDTH / 2.0f));
        int y = static_cast<int>(std::floor(worldPos.z + (float)GRID_HEIGHT / 2.0f));
        return { x, y };
    }

private:
    std::vector<std::vector<ecs::Entity>> m_Grid;
};