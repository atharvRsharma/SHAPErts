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
    float resourcesPerSecond = 2.0f;
};

struct HealthComponent {
    float currentHP = 100.0f;
    float maxHP = 100.0f;
};

struct EnemyComponent {};

struct ProjectileComponent { 
    glm::vec3 velocity;
    float damage;
};

struct BombComponent {
    float triggerRadius = 1.5f; 
    float blastRadius = 2.5f;
    float damage = 50.0f;
};

struct TurretAIComponent {
    float range = 8.0f;
    float fovDegrees = 90.0f;
    float turnSpeed = 90.0f; // degrees per second
    ecs::Entity currentTarget = ecs::MAX_ENTITIES;

    // "Durability" cost: damage to self per burst
    float selfDamagePerBurst = 5.0f;

    // Burst-fire logic
    float fireCooldown = 0.0f;
    float burstDelay = 0.1f;
    float reloadTime = 2.0f;
    int shotsInBurst = 5;
    int currentAmmo = 5;
};

// --- UPDATED: Holds AI state for an enemy ---
struct MovementComponent {
    float speed = 2.0f;
    std::vector<glm::vec3> path;
    int currentPathIndex = 0;

    // --- THIS IS THE CoC "LOCK-ON" ---
    ecs::Entity targetEntity = ecs::MAX_ENTITIES; // The building this enemy is locked onto
    bool isAttacking = false;
    float attackRate = 1.0f;
    float attackCooldown = 0.0f;
};