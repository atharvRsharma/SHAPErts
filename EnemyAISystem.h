#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h"
#include "GridSystem.h"
#include "Pathfinder.h"
#include <glm/glm.hpp>
#include <optional>
#include <iostream> 
#include <random> // <-- NEW: For random targeting

class EnemyAISystem : public ecs::System {
public:
    void Init(GridSystem* gridSystem) {
        m_GridSystem = gridSystem;
        
        // --- NEW: Initialize random number generator ---
        m_RandomEngine.seed(std::random_device()());
    }
    
    void Update(float dt, ecs::Registry* registry) {
        
        // 1. Find all player buildings (potential targets)
        std::vector<ecs::Entity> targets;
        // We must loop over ALL entities, as RenderSystem won't see them
        // if they don't have a signature. This is a safer way.
        for (ecs::Entity entity = 0; entity < ecs::MAX_ENTITIES; ++entity) {
            if (registry->HasComponent<BuildingComponent>(entity)) {
                targets.push_back(entity);
            }
        }
        if (targets.empty()) return; // No targets to attack

        // 2. Loop over all enemies
        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            auto& movement = m_Registry->GetComponent<MovementComponent>(entity);
            
            // 3. AI logic: If not on a path, find a new one
            if (movement.path.empty() || movement.currentPathIndex >= movement.path.size()) 
            {
                // --- THIS IS THE PILLAGING FIX ---
                // Pick a random target from the list
                std::uniform_int_distribution<int> dist(0, targets.size() - 1);
                ecs::Entity randomTarget = targets[dist(m_RandomEngine)];
                // --- END OF FIX ---

                // 4. We have a target. Ask A* for a path.
                glm::ivec2 startTile = m_GridSystem->WorldToGrid(transform.position);
                glm::ivec2 endTile = m_GridSystem->WorldToGrid(registry->GetComponent<TransformComponent>(randomTarget).position);

                movement.path = Pathfinder::FindPath(m_GridSystem, startTile, endTile);
                movement.currentPathIndex = 0;
                
                if (!movement.path.empty()) {
                    std::cout << "Enemy " << entity << " is pillaging target " << randomTarget << ". Steps: " << movement.path.size() << std::endl;
                } else {
                    std::cout << "Enemy " << entity << " COULD NOT FIND PATH from " << startTile.x << "," << startTile.y << " to " << endTile.x << "," << endTile.y << std::endl;
                }
            }
        }
    }

private:
    GridSystem* m_GridSystem;
    std::mt19937 m_RandomEngine; // --- NEW ---
};