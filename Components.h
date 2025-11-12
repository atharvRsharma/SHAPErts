// All elements are generic placeholders for testing purposes.
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <string>

// --- NEW MESH ENUM ---
enum class MeshType {
    None,
    Quad,
    Cube,
    Unit_Soldier // Future: A complex, multi-part mesh
};

// --- Enum Definitions ---
enum class BuildingType {
    Base,
    ResourceNode,
    Turret
};

// --- Component Structs ---
struct TransformComponent {
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f }; // --- VEC3 ---
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f }; // --- VEC3 (Euler angles) ---
};

// --- RENAMED: Just for color now ---
struct RenderComponent {
    glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

// --- NEW: Tells the renderer WHAT to draw ---
struct MeshComponent {
    MeshType type = MeshType::None;
};

// --- This is now a logical component, not a renderable one ---
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