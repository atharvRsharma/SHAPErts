// All elements are generic placeholders for testing purposes.
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <string>

// --- Enum Definitions ---
enum class BuildingType {
    Base,
    ResourceNode,
    Turret
};

// --- Component Structs ---
// Note: These are Plain Old Data (POD) structs.
// They contain *only* data, no logic.

// Basic spatial component
struct TransformComponent {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec2 scale = { 1.0f, 1.0f };
    float rotation = 0.0f; // in radians
};

// Data needed for rendering
struct RenderComponent {
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // We could add shader ID, texture ID, mesh ID, etc.
};

// For entities that are part of the grid
struct GridTileComponent {
    int x;
    int y;
    bool isOccupied = false;
};

// For entities that can be selected
struct SelectableComponent {
    bool isSelected = false;
};

// For entities with health (Units, Buildings)
struct HealthComponent {
    float currentHP = 100.0f;
    float maxHP = 100.0f;
};

// For buildings that generate resources
struct ResourceGeneratorComponent {
    float resourcesPerSecond = 1.0f;
};

// For entities that are buildings
struct BuildingComponent {
    BuildingType type;
};

// For entities that are units
struct UnitComponent {
    float attackDamage = 10.0f;
    float attackRadius = 2.0f;
    float attackSpeed = 1.0f; // attacks per second
    float timeSinceLastAttack = 0.0f;
};

// For entities that can move
struct MovementComponent {
    glm::vec2 targetPosition;
    std::vector<glm::vec2> path;
    float speed = 2.0f; // units per second
    bool isMoving = false;
};