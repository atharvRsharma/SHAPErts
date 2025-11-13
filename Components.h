#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <string>

enum class MeshType {
    None,
    Quad,
    Cube,
    Pyramid,
    Turret,
    Sphere
};

enum class BuildingType {
    None,
    Base,
    ResourceNode,
    Turret,
    Bomb,
    Enemy
};

struct TransformComponent {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
};

struct RenderComponent {
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct MeshComponent {
    MeshType type = MeshType::None;
};

struct BuildingComponent {
    BuildingType type = BuildingType::None;
};

struct GridTileComponent {
    int x;
    int y;
    bool isOccupied = false;
};

struct SelectableComponent {
    bool isSelected = false;
};


struct GhostComponent {};

struct ResourceGeneratorComponent {
    float resourcesPerSecond = 1.0f;
};

struct HealthComponent {
    float currentHP = 100.0f;
    float maxHP = 100.0f;
};

struct EnemyComponent {};

struct MovementComponent {
    glm::vec3 targetPosition;
    float speed = 2.0f;
    bool isMoving = false;
};