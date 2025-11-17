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
    Sphere,
    Capsule
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
    int resourcesPerSecond = 2;
};

struct HealthComponent {
    int currentHP = 100;
    int maxHP = 100;
};

struct EnemyComponent {};

struct ProjectileComponent { 
    glm::vec3 velocity;
    int damage;
};

struct BombComponent {
    float triggerRadius = 1.5f; 
    float blastRadius = 2.5f;
    int damage = 50;
};

struct TurretAIComponent {
    float range = 8.0f;
    float fovDegrees = 90.0f;
    float turnSpeed = 90.0f; //degrees per second
    ecs::Entity currentTarget = ecs::MAX_ENTITIES;

    //self inflicted damage
    int selfDamagePerBurst = 5;

    //burst fire
    float fireCooldown = 0.0f;
    float burstDelay = 0.1f;
    float reloadTime = 2.0f;
    int shotsInBurst = 5;
    int currentAmmo = 5;
};

struct MovementComponent {
    float speed = 2.0f;
    std::vector<glm::vec3> path;
    int currentPathIndex = 0;

    //lock on functionality
    ecs::Entity targetEntity = ecs::MAX_ENTITIES; //actual building the enemy latched onto
    bool isAttacking = false;
    short attackRate = 1;
    float attackCooldown = 0.0f;
};

struct CollisionComponent {
    float radius = 0.5f;
};