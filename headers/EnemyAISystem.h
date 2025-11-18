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
    }

    void Update(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allRenderableEntities) {

        m_AiRepathTimer -= dt;
        if (m_AiRepathTimer > 0.0f) {
            return; 
        }
        m_AiRepathTimer = 1.0f; 

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

            if (!movement.isAttacking && movement.targetEntity == ecs::MAX_ENTITIES)
            {
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

                movement.targetEntity = closestTarget;
                std::cout << "AI: Enemy " << entity << " acquired new target " << closestTarget << "!" << std::endl;
            }
        }
    }

private:
    GridSystem* m_GridSystem;
    std::mt19937 m_RandomEngine;
    float m_AiRepathTimer = 0.0f;
};