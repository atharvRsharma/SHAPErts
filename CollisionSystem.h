#pragma once


#include "ECS.h"
#include "Components.h"
#include <glm/glm.hpp>
#include <vector>

class CollisionSystem : public ecs::System {
public:
    void Update(float dt, GridSystem* gridSystem) {
        // m_Entities now contains *all* entities with a CollisionComponent
        // (buildings and enemies)
        std::vector<ecs::Entity> collidables(m_Entities.begin(), m_Entities.end());

        for (size_t i = 0; i < collidables.size(); ++i) {
            auto entityA = collidables[i];
            // Only enemies can be "pushed"
            if (!m_Registry->HasComponent<EnemyComponent>(entityA)) continue;

            auto& transformA = m_Registry->GetComponent<TransformComponent>(entityA);
            auto& collisionA = m_Registry->GetComponent<CollisionComponent>(entityA);

            for (size_t j = 0; j < collidables.size(); ++j) {
                if (i == j) continue; // Don't check against self

                auto entityB = collidables[j];
                auto& transformB = m_Registry->GetComponent<TransformComponent>(entityB);
                auto& collisionB = m_Registry->GetComponent<CollisionComponent>(entityB);

                // --- 2D Collision Check (on XZ plane) ---
                glm::vec2 posA = { transformA.position.x, transformA.position.z };
                glm::vec2 posB = { transformB.position.x, transformB.position.z };
                float dist = glm::distance(posA, posB);

                // Get combined radius. Buildings are scaled, so use scale.
                float radiusA = collisionA.radius;
                float radiusB = collisionB.radius;

                // This is a simple AABB-to-Circle check
                if (m_Registry->HasComponent<BuildingComponent>(entityB)) {
                    // Use building footprint as "radius"
                    radiusB = (transformB.scale.x + transformB.scale.z) / 4.0f; // Average
                }

                float combinedRadius = radiusA + radiusB;

                if (dist < combinedRadius) {
                    // Collision!
                    float overlap = combinedRadius - dist;
                    glm::vec2 pushDir = (dist > 0.001f) ? glm::normalize(posA - posB) : glm::vec2(1, 0);

                    // Push enemy A away
                    glm::vec2 push = pushDir * overlap;

                    // --- THIS IS THE FIX ---
                    // Check if the *new* position is walkable before pushing
                    glm::vec3 newPos = transformA.position + glm::vec3(push.x, 0.0f, push.y);
                    glm::ivec2 newGridPos = gridSystem->WorldToGrid(newPos);

                    if (gridSystem->IsWalkable(newGridPos.x, newGridPos.y)) {
                        transformA.position = newPos;
                    }
                    // --- END OF FIX ---
                }
            }
        }
    }
};