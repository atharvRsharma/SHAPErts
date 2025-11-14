#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h"
#include <glm/glm.hpp>

class MovementSystem : public ecs::System {
public:
    void Update(float dt) {
        // Loop over all entities that have a Transform and Movement component
        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            auto& movement = m_Registry->GetComponent<MovementComponent>(entity);

            // --- A* PATH FOLLOWING ---
            if (movement.path.empty() || movement.currentPathIndex >= movement.path.size()) {
                continue; // No path, or path is finished
            }

            // Get the next waypoint
            glm::vec3 targetWaypoint = movement.path[movement.currentPathIndex];

            // --- HACK: We ignore Y for pathfinding ---
            targetWaypoint.y = transform.position.y;

            glm::vec3 direction = glm::normalize(targetWaypoint - transform.position);

            // Check if we're close enough to the waypoint
            if (glm::distance(transform.position, targetWaypoint) < 0.1f) {
                movement.currentPathIndex++; // Move to next waypoint
            }
            else {
                // Keep moving
                transform.position += direction * movement.speed * dt;
            }
        }
    }
};