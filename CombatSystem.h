#pragma once


#include "ECS.h"
#include "Components.h"
#include "BalanceSystem.h"
#include "ResourceSystem.h"
#include "GridSystem.h"
#include <iostream>
#include <optional>
#include <algorithm>
#include <set> 

class CombatSystem : public ecs::System {
public:
    void Init(BalanceSystem* balance, ResourceSystem* resources, GridSystem* grid) {
        m_BalanceSystem = balance;
        m_ResourceSystem = resources;
        m_GridSystem = grid;
    }

    void Update(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies, const std::set<ecs::Entity>& allRenderableEntities) {

       
        UpdateTurrets(dt, registry, allEnemies);

        UpdateEnemies(dt, registry, allEnemies);
        UpdateBombs(dt, registry, allEnemies, allRenderableEntities);
        CheckForDeaths(registry, allRenderableEntities);
    }

private:
    BalanceSystem* m_BalanceSystem;
    ResourceSystem* m_ResourceSystem;
    GridSystem* m_GridSystem;

    void UpdateTurrets(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies) {
        for (auto const& entity : m_Entities) {
            auto& turret = m_Registry->GetComponent<TurretAIComponent>(entity);
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            turret.fireCooldown -= dt;

            FindTargetForTurret(registry, allEnemies, entity);

            if (IsTargetValid(registry, turret.currentTarget, transform, turret.range)) {
                auto& targetTransform = registry->GetComponent<TransformComponent>(turret.currentTarget);

                float aimDifference = RotateTurret(transform, turret, targetTransform.position, dt);

                if (turret.fireCooldown <= 0.0f && std::abs(aimDifference) < 5.0f &&
                    HasLineOfSight(transform.position, targetTransform.position))
                {
                    FireAtTarget(registry, transform.position, targetTransform.position);

                    if (turret.currentAmmo > 0) {
                        turret.currentAmmo--;
                        turret.fireCooldown = turret.burstDelay;
                    }
                    if (turret.currentAmmo == 0) {
                        turret.fireCooldown = turret.reloadTime;
                        turret.currentAmmo = turret.shotsInBurst;
                        auto& selfHealth = registry->GetComponent<HealthComponent>(entity);
                        selfHealth.currentHP -= turret.selfDamagePerBurst;
                    }
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
        std::optional<ecs::Entity> bestTarget = std::nullopt;

        for (auto const& enemyEntity : allEnemies) {
            if (!registry->HasComponent<HealthComponent>(enemyEntity)) continue;

            auto& enemyTransform = registry->GetComponent<TransformComponent>(enemyEntity);
            float dist = glm::distance(transform.position, enemyTransform.position);

            if (dist < closestDist && IsTargetInFOV(transform, enemyTransform, turret)) {
                closestDist = dist;
                bestTarget = enemyEntity;
            }
        }

        if (bestTarget) {
            turret.currentTarget = *bestTarget;
        }
        else {
            turret.currentTarget = ecs::MAX_ENTITIES;
        }
    }

    float RotateTurret(TransformComponent& turretTransform, TurretAIComponent& turret, const glm::vec3& targetPos, float dt) {
        glm::vec3 directionToTarget = glm::normalize(targetPos - turretTransform.position);

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

    bool HasLineOfSight(glm::vec3 start, glm::vec3 end) {
        glm::ivec2 startTile = m_GridSystem->WorldToGrid(start);
        glm::ivec2 endTile = m_GridSystem->WorldToGrid(end);

        //bresenhams algo to draw a line bw turr and target(los)
        int x0 = startTile.x, y0 = startTile.y;
        int x1 = endTile.x, y1 = endTile.y;
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
            
            if (x0 != startTile.x || y0 != startTile.y) {
                if (m_GridSystem->IsTileOccupied(x0, y0)) {
                    return false;
                }
            }
            if (x0 == x1 && y0 == y1) break; 
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
        return true; //no obstructions
    }

    void FireAtTarget(ecs::Registry* registry, glm::vec3 turretPos, glm::vec3 targetPos) {
        auto bullet = registry->CreateEntity();

        //spawn bullet above the turret
        turretPos.y += 0.5f;

        registry->AddComponent(bullet, TransformComponent{
            turretPos,
            {0.2f, 0.2f, 0.2f},
            {0,0,0}
            });
        registry->AddComponent(bullet, RenderComponent{ {1.0f, 0.5f, 0.0f, 1.0f} }); // Orange
        registry->AddComponent(bullet, MeshComponent{ MeshType::Sphere });

        glm::vec3 velocity = glm::normalize(targetPos - turretPos) * 15.0f; //15 units per sec
        registry->AddComponent(bullet, ProjectileComponent{ velocity, 4 }); //4 damage
    }

    void UpdateEnemies(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies) {
        for (auto const& entity : allEnemies) {
            auto& movement = registry->GetComponent<MovementComponent>(entity);
            if (movement.isAttacking) {
                movement.attackCooldown -= dt;
                if (movement.attackCooldown <= 0.0f) {
                    if (registry->HasComponent<HealthComponent>(movement.targetEntity)) {
                        auto& targetHealth = registry->GetComponent<HealthComponent>(movement.targetEntity);
                        targetHealth.currentHP -= 5;
                        std::cout << "Enemy " << entity << " attacks " << movement.targetEntity << "! HP: " << targetHealth.currentHP << std::endl;
                    }
                    movement.attackCooldown = movement.attackRate;
                }
            }
        }
    }

    void UpdateBombs(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies, const std::set<ecs::Entity>& allRenderableEntities) {
        for (auto const& entity : allRenderableEntities) {
            if (!registry->HasComponent<BombComponent>(entity)) continue;

            auto& bomb = registry->GetComponent<BombComponent>(entity);
            auto& transform = registry->GetComponent<TransformComponent>(entity);

            for (auto const& enemy : allEnemies) {
                auto& enemyT = registry->GetComponent<TransformComponent>(enemy);
                if (glm::distance(transform.position, enemyT.position) < bomb.triggerRadius) {

                    registry->GetComponent<HealthComponent>(entity).currentHP = 0;
                    break;
                }
            }
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
        if (!registry->HasComponent<HealthComponent>(entity)) return; 

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

            if (building.type == BuildingType::Base) {
                std::cout << "GAME OVER: Your base was destroyed!" << std::endl;
            }

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

            //check if destroyed buildignd were bombs
            if (registry->HasComponent<BombComponent>(entity)) {
                std::cout << "BOMB " << entity << " EXPLODES!" << std::endl;
                auto& bomb = registry->GetComponent<BombComponent>(entity);

                //find all surrounding entities with health and damage em
                for (ecs::Entity target = 0; target < ecs::MAX_ENTITIES; ++target) {
                    if (!registry->HasComponent<HealthComponent>(target)) continue;
                    if (target == entity) continue; 

                    auto& targetT = registry->GetComponent<TransformComponent>(target);
                    if (glm::distance(transform.position, targetT.position) < bomb.blastRadius) {
                        auto& health = registry->GetComponent<HealthComponent>(target);
                        health.currentHP -= bomb.damage;
                    }
                }
            }
        }

        registry->DestroyEntity(entity);
    }
};