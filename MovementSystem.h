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

            if (!movement.isMoving) continue;

            // Simple "seek" logic
            glm::vec3 direction = glm::normalize(movement.targetPosition - transform.position);
            
            // Check if we're close enough
            if (glm::distance(transform.position, movement.targetPosition) < 0.1f) {
                movement.isMoving = false;
                continue;
            }

            transform.position += direction * movement.speed * dt;
        }
    }
};