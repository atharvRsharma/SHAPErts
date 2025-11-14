#pragma once


#include "ECS.h"
#include "Components.h"
#include "BalanceSystem.h"
#include "ResourceSystem.h"
#include "GridSystem.h"
#include <iostream>
#include <optional>
#include <algorithm>
#include <set> // For queueing explosions

class CombatSystem : public ecs::System {
public:
    void Init(BalanceSystem* balance, ResourceSystem* resources, GridSystem* grid) {
        m_BalanceSystem = balance;
        m_ResourceSystem = resources;
        m_GridSystem = grid;
    }

    void Update(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies, const std::set<ecs::Entity>& allRenderableEntities) {

        m_ExplosionQueue.clear();

        // --- 1. Turret AI Logic ---
        UpdateTurrets(dt, registry, allEnemies);

        // --- (Enemy, Bomb, Explosion, Death logic is all unchanged) ---
        UpdateEnemies(dt, registry, allEnemies);
        UpdateBombs(dt, registry, allEnemies, allRenderableEntities);
        ProcessExplosions(registry, allRenderableEntities);
        CheckForDeaths(registry, allRenderableEntities);
    }

private:
    BalanceSystem* m_BalanceSystem;
    ResourceSystem* m_ResourceSystem;
    GridSystem* m_GridSystem;
    std::set<ecs::Entity> m_ExplosionQueue; // For chain reactions

    void UpdateTurrets(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies) {
        for (auto const& entity : m_Entities) {
            auto& turret = m_Registry->GetComponent<TurretAIComponent>(entity);
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            turret.fireCooldown -= dt;

            if (turret.fireCooldown > 0.0f) continue;

            if (!IsTargetValid(registry, turret.currentTarget, transform, turret.range)) {
                turret.currentTarget = ecs::MAX_ENTITIES;
                FindTargetForTurret(registry, allEnemies, entity);
            }

            if (turret.currentTarget != ecs::MAX_ENTITIES) {
                auto& targetTransform = registry->GetComponent<TransformComponent>(turret.currentTarget);

                float aimDifference = RotateTurret(transform, turret, targetTransform.position, dt);

                if (std::abs(aimDifference) < 5.0f) { // Aimed
                    FireAtTarget(registry, transform.position, targetTransform.position);

                    // --- THIS IS THE "DURABILITY" FIX ---
                    if (turret.currentAmmo > 0) {
                        turret.currentAmmo--;
                        turret.fireCooldown = turret.burstDelay;
                    }
                    if (turret.currentAmmo == 0) {
                        // Just finished a burst, reload AND take damage
                        turret.fireCooldown = turret.reloadTime;
                        turret.currentAmmo = turret.shotsInBurst;

                        // Turret damages itself
                        auto& selfHealth = registry->GetComponent<HealthComponent>(entity);
                        selfHealth.currentHP -= turret.selfDamagePerBurst;
                        std::cout << "Turret " << entity << " takes durability damage! HP: " << selfHealth.currentHP << std::endl;
                    }
                    // --- END OF FIX ---
                }
            }
        }
    }

    bool IsTargetValid(ecs::Registry* registry, ecs::Entity target, const TransformComponent& turretTransform, float range) {
        if (target == ecs::MAX_ENTITIES) return false;
        if (!registry->HasComponent<HealthComponent>(target)) return false;
        auto& targetTransform = registry->GetComponent<TransformComponent>(target);
        return glm::distance(turretTransform.position, targetTransform.position) <= range;
    }

    void FindTargetForTurret(ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies, ecs::Entity turretEntity) {
        auto& turret = m_Registry->GetComponent<TurretAIComponent>(turretEntity);
        auto& transform = m_Registry->GetComponent<TransformComponent>(turretEntity);
        float closestDist = turret.range + 1.0f;

        for (auto const& enemyEntity : allEnemies) {
            // Check if enemy is valid
            if (!registry->HasComponent<HealthComponent>(enemyEntity)) continue;

            auto& enemyTransform = registry->GetComponent<TransformComponent>(enemyEntity);
            float dist = glm::distance(transform.position, enemyTransform.position);

            // --- THIS IS THE FIX (PART 2) ---
            // Only consider targets inside the FOV
            if (dist < closestDist && IsTargetInFOV(transform, enemyTransform, turret)) {
                closestDist = dist;
                turret.currentTarget = enemyEntity;
            }
            // --- END OF FIX ---
        }
    }

    float RotateTurret(TransformComponent& turretTransform, TurretAIComponent& turret, const glm::vec3& targetPos, float dt) {
        glm::vec3 directionToTarget = glm::normalize(targetPos - turretTransform.position);

        // --- YOUR -90.f FIX IS APPLIED HERE ---
        float targetYaw = glm::degrees(atan2(directionToTarget.x, directionToTarget.z)) - 90.f;

        float currentYaw = turretTransform.rotation.y;
        float diff = targetYaw - currentYaw;
        while (diff < -180.0f) diff += 360.0f;
        while (diff > 180.0f) diff -= 360.0f;

        float turn = std::clamp(diff, -turret.turnSpeed * dt, turret.turnSpeed * dt);
        turretTransform.rotation.y += turn;

        return diff - turn; // Return the remaining difference
    }

    bool IsTargetInFOV(const TransformComponent& turretTransform, const TransformComponent& targetTransform, const TurretAIComponent& turret) {

        float yawRad = glm::radians(turretTransform.rotation.y);
        glm::vec3 turretForward = glm::vec3(cos(yawRad), 0.0f, sin(yawRad));

        glm::vec3 toTarget = glm::normalize(targetTransform.position - turretTransform.position);
        float angle = glm::degrees(acos(glm::dot(turretForward, toTarget)));

        return (angle < turret.fovDegrees / 2.0f);
    }

    void FireAtTarget(ecs::Registry* registry, glm::vec3 turretPos, glm::vec3 targetPos) {
        auto bullet = registry->CreateEntity();

        // Spawn bullet above the turret
        turretPos.y += 0.5f;

        registry->AddComponent(bullet, TransformComponent{
            turretPos,
            {0.2f, 0.2f, 0.2f}, // Small sphere
            {0,0,0}
            });
        registry->AddComponent(bullet, RenderComponent{ {1.0f, 0.5f, 0.0f, 1.0f} }); // Orange
        registry->AddComponent(bullet, MeshComponent{ MeshType::Sphere });

        glm::vec3 velocity = glm::normalize(targetPos - turretPos) * 15.0f; // 15 units/sec
        registry->AddComponent(bullet, ProjectileComponent{ velocity, 4.0f }); // 10 damage
    }

    void UpdateEnemies(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies) {
        for (auto const& entity : allEnemies) {
            auto& movement = registry->GetComponent<MovementComponent>(entity);
            if (movement.isAttacking) {
                movement.attackCooldown -= dt;
                if (movement.attackCooldown <= 0.0f) {
                    if (registry->HasComponent<HealthComponent>(movement.targetEntity)) {
                        auto& targetHealth = registry->GetComponent<HealthComponent>(movement.targetEntity);
                        targetHealth.currentHP -= 5.0f;
                        std::cout << "Enemy " << entity << " attacks " << movement.targetEntity << "! HP: " << targetHealth.currentHP << std::endl;
                    }
                    movement.attackCooldown = movement.attackRate;
                }
            }
        }
    }

    void UpdateBombs(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies, const std::set<ecs::Entity>& allRenderableEntities) {
        // Find all bombs
        for (auto const& entity : allRenderableEntities) { // <-- NO LONGER LOOPS TO 5000
            if (!registry->HasComponent<BombComponent>(entity)) continue;

            auto& bomb = registry->GetComponent<BombComponent>(entity);
            auto& transform = registry->GetComponent<TransformComponent>(entity);

            for (auto const& enemy : allEnemies) {
                auto& enemyT = registry->GetComponent<TransformComponent>(enemy);
                if (glm::distance(transform.position, enemyT.position) < bomb.triggerRadius) {
                    m_ExplosionQueue.insert(entity);
                    break;
                }
            }
        }
    }

    // --- 4. Explosion Logic ---
    void ProcessExplosions(ecs::Registry* registry, const std::set<ecs::Entity>& allRenderableEntities) {
        for (auto const& bombEntity : m_ExplosionQueue) {
            if (!registry->HasComponent<BombComponent>(bombEntity)) continue;

            auto& bomb = registry->GetComponent<BombComponent>(bombEntity);
            auto& transform = registry->GetComponent<TransformComponent>(bombEntity);
            std::cout << "BOMB " << bombEntity << " EXPLODES!" << std::endl;

            for (auto const& entity : allRenderableEntities) { // <-- NO LONGER LOOPS TO 5000
                if (!registry->HasComponent<HealthComponent>(entity)) continue;
                if (entity == bombEntity) continue;

                auto& targetT = registry->GetComponent<TransformComponent>(entity);
                if (glm::distance(transform.position, targetT.position) < bomb.blastRadius) {
                    auto& health = registry->GetComponent<HealthComponent>(entity);
                    health.currentHP -= bomb.damage;
                    std::cout << "BOMB hits " << entity << "! HP: " << health.currentHP << std::endl;

                    if (registry->HasComponent<BombComponent>(entity) && health.currentHP <= 0.0f) {
                        m_ExplosionQueue.insert(entity);
                    }
                }
            }
            registry->DestroyEntity(bombEntity);
        }
    }

    void CheckForDeaths(ecs::Registry* registry, const std::set<ecs::Entity>& allRenderableEntities) {
        std::set<ecs::Entity> deadEntities;
        for (auto const& entity : allRenderableEntities) {
            if (registry->HasComponent<HealthComponent>(entity)) {
                if (registry->GetComponent<HealthComponent>(entity).currentHP <= 0.0f) {
                    deadEntities.insert(entity);
                }
            }
        }

        for (auto const& entity : deadEntities) {
            OnEntityDied(registry, entity);
        }
    }

    void OnEntityDied(ecs::Registry* registry, ecs::Entity entity) {
        if (!registry->HasComponent<HealthComponent>(entity)) return; // Already dead

        if (registry->HasComponent<EnemyComponent>(entity)) {
            std::cout << "Enemy " << entity << " was destroyed!" << std::endl;
            m_BalanceSystem->OnEnemyKilled();
            m_ResourceSystem->AddResources(25);
        }
        else if (registry->HasComponent<BuildingComponent>(entity)) {
            std::cout << "Building " << entity << " was destroyed!" << std::endl;
            m_BalanceSystem->OnBuildingDestroyed();

            auto& transform = registry->GetComponent<TransformComponent>(entity);
            auto& building = registry->GetComponent<BuildingComponent>(entity);

            // If it was the base, game over!
            if (building.type == BuildingType::Base) {
                std::cout << "GAME OVER: Your base was destroyed!" << std::endl;
                // We'd set a game state here, but for now we'll just log
            }

            // Free up its grid tiles
            glm::ivec2 anchor = m_GridSystem->WorldToGrid(transform.position -
                glm::vec3((transform.scale.x / 2.0f) - 0.5f, 0.0f, (transform.scale.z / 2.0f) - 0.5f));

            int footprintX = static_cast<int>(transform.scale.x);
            int footprintZ = static_cast<int>(transform.scale.z);
            if (std::abs(transform.rotation.y - 90.0f) < 1.0f || std::abs(transform.rotation.y - 270.0f) < 1.0f) {
                std::swap(footprintX, footprintZ);
            }

            for (int x = 0; x < footprintX; ++x) {
                for (int z = 0; z < footprintZ; ++z) {
                    m_GridSystem->SetTileOccupied(anchor.x + x, anchor.y + z, false);
                }
            }
        }

        // This removes it from ALL systems, including RenderSystem
        registry->DestroyEntity(entity);
    }
};