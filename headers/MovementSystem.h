#pragma once

#include "ECS.h"
#include "Components.h"
#include "GridSystem.h" 
#include "Pathfinder.h" 
#include <glm/glm.hpp>
#include <iostream>

class MovementSystem : public ecs::System {
public:
    void Init(GridSystem* gridSystem) {
        m_GridSystem = gridSystem;
    }

    void Update(float dt, ecs::Registry* registry) {
        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            auto& movement = m_Registry->GetComponent<MovementComponent>(entity);

            if (movement.isAttacking) {
                continue;
            }

            if (movement.targetEntity == ecs::MAX_ENTITIES) {
                continue;
            }

            if (!registry->HasComponent<HealthComponent>(movement.targetEntity)) {
                movement.isAttacking = false;
                movement.targetEntity = ecs::MAX_ENTITIES;
                movement.path.clear();
                continue;
            }

            auto& targetTransform = registry->GetComponent<TransformComponent>(movement.targetEntity);

            float stopDistance = 0.5f;
            if (glm::distance(transform.position, targetTransform.position) <= stopDistance) {
                movement.isAttacking = true;
                movement.path.clear();
                std::cout << "Enemy " << entity << " reached target " << movement.targetEntity << " and is attacking!" << std::endl;
                continue;
            }

            if (movement.path.empty() || movement.currentPathIndex >= movement.path.size()) {
                glm::ivec2 startTile = m_GridSystem->WorldToGrid(transform.position);
                glm::ivec2 endTile = m_GridSystem->WorldToGrid(targetTransform.position);

                movement.path = Pathfinder::FindPath(m_GridSystem, startTile, endTile);
                movement.currentPathIndex = 0;

                if (movement.path.empty()) {
                    continue;
                }
            }

            glm::vec3 targetWaypoint = movement.path[movement.currentPathIndex];
            targetWaypoint.y = transform.position.y;

            glm::vec2 pos2D = { transform.position.x, transform.position.z };
            glm::vec2 target2D = { targetWaypoint.x, targetWaypoint.z };
            float distance = glm::distance(pos2D, target2D);

            if (distance < 0.1f) {
                movement.currentPathIndex++;
            }
            else {
                glm::vec3 direction = glm::normalize(targetWaypoint - transform.position);
                transform.position += direction * movement.speed * dt;
            }
        }
    }
private:
    GridSystem* m_GridSystem;
};