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
                continue; // Stop moving if we're attacking
            }

            if (movement.path.empty() || movement.currentPathIndex >= movement.path.size()) {
                continue; // No path
            }

            // --- THIS IS THE "STOP 1-2 BLOCKS AWAY" FIX ---

            // Check if we're on the *last step* of the path
            if (movement.currentPathIndex == movement.path.size() - 1)
            {
                // We're moving towards the final target
                auto& targetTransform = m_Registry->GetComponent<TransformComponent>(movement.targetEntity);

                // Calculate "stop distance" (1.5 units)
                float stopDistance = 1.5f;

                if (glm::distance(transform.position, targetTransform.position) <= stopDistance) {
                    // We're close enough! Stop moving and start attacking.
                    movement.isAttacking = true;
                    movement.path.clear();
                    std::cout << "Enemy " << entity << " reached target " << movement.targetEntity << " and is attacking!" << std::endl;
                    continue;
                }
            }
            // --- END OF FIX ---


            // --- (Path following logic) ---
            glm::vec3 targetWaypoint = movement.path[movement.currentPathIndex];

            glm::vec2 pos2D = { transform.position.x, transform.position.z };
            glm::vec2 target2D = { targetWaypoint.x, targetWaypoint.z };
            float distance = glm::distance(pos2D, target2D);

            targetWaypoint.y = transform.position.y;

            glm::vec3 direction = glm::normalize(targetWaypoint - transform.position);

            if (distance < 0.1f) {
                movement.currentPathIndex++; // Move to next waypoint
            }
            else {
                transform.position += direction * movement.speed * dt;
            }
        }
    }
};