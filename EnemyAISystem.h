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


            //check is target dead, if yes move on
            if (movement.targetEntity != ecs::MAX_ENTITIES && !registry->HasComponent<HealthComponent>(movement.targetEntity)) {
                movement.targetEntity = ecs::MAX_ENTITIES; // It's dead, find a new one
                movement.isAttacking = false;
                movement.path.clear();
            }

            //if idle, try to find and engage target
            bool isIdle = (movement.path.empty() || movement.currentPathIndex >= movement.path.size());
            if (movement.targetEntity == ecs::MAX_ENTITIES && isIdle && doRepath)
            {
                //pillaging logic, lock onto target and go to town
                std::uniform_int_distribution<int> dist(0, targets.size() - 1);
                ecs::Entity randomTarget = targets[dist(m_RandomEngine)];
                movement.targetEntity = randomTarget; // <-- LOCK ON

                glm::ivec2 startTile = m_GridSystem->WorldToGrid(transform.position);
                glm::ivec2 endTile = m_GridSystem->WorldToGrid(registry->GetComponent<TransformComponent>(randomTarget).position);

                movement.path = Pathfinder::FindPath(m_GridSystem, startTile, endTile);
                movement.currentPathIndex = 0;

                if (movement.path.empty()) {
                    std::cout << "Enemy " << entity << " COULD NOT FIND PATH" << std::endl;
                }
                else {
                    std::cout << "Enemy " << entity << " acquired new target " << randomTarget << "!" << std::endl;
                }
            }
        }
    }

private:
    GridSystem* m_GridSystem;
    std::mt19937 m_RandomEngine;
    float m_AiRepathTimer = 0.0f;
};