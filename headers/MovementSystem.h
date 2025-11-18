#pragma once

#include "ECS.h"
#include "Components.h"
#include <glm/glm.hpp>
#include <iostream>

class MovementSystem : public ecs::System {
public:
    void Update(float dt) {
        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            auto& movement = m_Registry->GetComponent<MovementComponent>(entity);

            if (movement.isAttacking) {
                continue; //if attacking->stop moving
            }

            if (movement.path.empty() || movement.currentPathIndex >= movement.path.size()) {
                continue; //no path
            }

            if (movement.targetEntity != ecs::MAX_ENTITIES &&
                m_Registry->HasComponent<TransformComponent>(movement.targetEntity) &&
                movement.currentPathIndex == movement.path.size() - 1) 
            {
                auto& targetTransform = m_Registry->GetComponent<TransformComponent>(movement.targetEntity);
                
                
                glm::vec2 pos2D = {transform.position.x, transform.position.z};
                glm::vec2 target2D = {targetTransform.position.x, targetTransform.position.z};
                float stopDistance = 1.5f; 
                
                if (glm::distance(pos2D, target2D) <= stopDistance) {
                    movement.isAttacking = true;
                    movement.path.clear();
                    std::cout << "Enemy " << entity << " reached target " << movement.targetEntity << " and is attacking!" << std::endl;
                    continue;
                }
            }


            glm::vec3 targetWaypoint = movement.path[movement.currentPathIndex];
            glm::vec2 pos2D = {transform.position.x, transform.position.z};
            glm::vec2 target2D = {targetWaypoint.x, targetWaypoint.z};
            float distance = glm::distance(pos2D, target2D);
            
            targetWaypoint.y = transform.position.y;
            glm::vec3 direction = glm::normalize(targetWaypoint - transform.position);

            if (distance < 0.1f) {
                movement.currentPathIndex++; 
            } else {
                transform.position += direction * movement.speed * dt;
            }
        }
    }
};