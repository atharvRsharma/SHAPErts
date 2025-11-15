#pragma once

#include "ECS.h"
#include "Components.h"
#include "GridSystem.h"
#include "Pathfinder.h"
#include <glm/glm.hpp>
#include <optional>
#include <iostream> 
#include <random> 

class EnemyAISystem : public ecs::System {
public:
    void Init(GridSystem* gridSystem) {
        m_GridSystem = gridSystem;
        m_RandomEngine.seed(std::random_device()());
    }

    void Update(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allRenderableEntities) {

        m_AiRepathTimer -= dt;
        bool doRepath = (m_AiRepathTimer <= 0.0f);
        if (doRepath) {
            m_AiRepathTimer = 1.0f;
        }

        std::vector<ecs::Entity> targets;
        for (auto const& entity : allRenderableEntities) {
            if (registry->HasComponent<BuildingComponent>(entity)) {
                targets.push_back(entity);
            }
        }
        if (targets.empty()) return;

        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            auto& movement = m_Registry->GetComponent<MovementComponent>(entity);

            // 1. Check if our current target is dead
            if (movement.targetEntity != ecs::MAX_ENTITIES && !registry->HasComponent<HealthComponent>(movement.targetEntity)) {
                movement.targetEntity = ecs::MAX_ENTITIES;
                movement.isAttacking = false;
                movement.path.clear();
            }

            bool isIdle = movement.path.empty() || movement.currentPathIndex >= movement.path.size();

            // --- THIS IS THE "CLOSEST" TARGET FIX ---
            // 2. If we are idle (no target, not attacking), find a new, *closest* target
            if (movement.targetEntity == ecs::MAX_ENTITIES && isIdle && doRepath)
            {
                // "Pillaging" AI: Find Closest Target
                ecs::Entity closestTarget = targets[0];
                float minDistance = FLT_MAX;

                for (auto const& targetEntity : targets) {
                    auto& targetTransform = registry->GetComponent<TransformComponent>(targetEntity);
                    float dist = glm::distance(transform.position, targetTransform.position);
                    if (dist < minDistance) {
                        minDistance = dist;
                        closestTarget = targetEntity;
                    }
                }
                movement.targetEntity = closestTarget; // <-- LOCK ON
                // --- END OF FIX ---

                glm::ivec2 startTile = m_GridSystem->WorldToGrid(transform.position);
                glm::ivec2 endTile = m_GridSystem->WorldToGrid(registry->GetComponent<TransformComponent>(closestTarget).position);

                movement.path = Pathfinder::FindPath(m_GridSystem, startTile, endTile);
                movement.currentPathIndex = 0;

                if (movement.path.empty()) {
                    std::cout << "Enemy " << entity << " COULD NOT FIND PATH" << std::endl;
                }
                else {
                    std::cout << "Enemy " << entity << " acquired new target " << closestTarget << "!" << std::endl;
                }
            }
        }
    }

private:
    GridSystem* m_GridSystem;
    std::mt19937 m_RandomEngine;
    float m_AiRepathTimer = 0.0f;
};