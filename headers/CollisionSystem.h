#pragma once


#include "ECS.h"
#include "Components.h"
#include <glm/glm.hpp>
#include <vector>

class CollisionSystem : public ecs::System {
public:
    void Update(float dt, GridSystem* gridSystem) {
        std::vector<ecs::Entity> collidables(m_Entities.begin(), m_Entities.end());

        for (size_t i = 0; i < collidables.size(); ++i) {
            auto entityA = collidables[i];
            if (!m_Registry->HasComponent<EnemyComponent>(entityA)) continue;

            auto& transformA = m_Registry->GetComponent<TransformComponent>(entityA);
            auto& collisionA = m_Registry->GetComponent<CollisionComponent>(entityA);

            for (size_t j = 0; j < collidables.size(); ++j) {
                if (i == j) continue; 

                auto entityB = collidables[j];
                auto& transformB = m_Registry->GetComponent<TransformComponent>(entityB);
                auto& collisionB = m_Registry->GetComponent<CollisionComponent>(entityB);

                glm::vec2 posA = { transformA.position.x, transformA.position.z };
                glm::vec2 posB = { transformB.position.x, transformB.position.z };
                float dist = glm::distance(posA, posB);

                float radiusA = collisionA.radius;
                float radiusB = collisionB.radius;

                if (m_Registry->HasComponent<BuildingComponent>(entityB)) {
                    radiusB = (transformB.scale.x + transformB.scale.z) / 4.0f;
                }

                float combinedRadius = radiusA + radiusB;

                if (dist < combinedRadius) {
                    float overlap = combinedRadius - dist;
                    glm::vec2 pushDir = (dist > 0.001f) ? glm::normalize(posA - posB) : glm::vec2(1, 0);

                    glm::vec2 push = pushDir * overlap;

                    glm::vec3 newPos = transformA.position + glm::vec3(push.x, 0.0f, push.y);
                    glm::ivec2 newGridPos = gridSystem->WorldToGrid(newPos);

                    if (gridSystem->IsWalkable(newGridPos.x, newGridPos.y)) {
                        transformA.position = newPos;
                    }
                }
            }
        }
    }
};