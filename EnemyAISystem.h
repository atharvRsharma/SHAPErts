#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h"
#include <glm/glm.hpp>

class EnemyAISystem : public ecs::System {
public:
    void Update(float dt, const glm::vec3& basePosition) {
        // Loop over all entities that have an Enemy and Movement component
        for (auto const& entity : m_Entities) {
            auto& movement = m_Registry->GetComponent<MovementComponent>(entity);

            // --- This is the AI logic ---
            // "Attack the base!"
            movement.targetPosition = basePosition;
            movement.isMoving = true;
        }
    }
};