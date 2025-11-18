#pragma once


#include "ECS.h"
#include "Components.h"
#include <glm/glm.hpp>
#include <set>

class ProjectileSystem : public ecs::System {
public:
    void Update(float dt, ecs::Registry* registry, const std::set<ecs::Entity>& allEnemies) {
        std::set<ecs::Entity> bulletsToDestroy;

        
        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);
            auto& projectile = m_Registry->GetComponent<ProjectileComponent>(entity);

            transform.position += projectile.velocity * dt;

            
            for (auto const& enemy : allEnemies) {
                auto& enemyT = registry->GetComponent<TransformComponent>(enemy);

               
                if (glm::distance(transform.position, enemyT.position) < 1.0f) {
                    
                    auto& health = registry->GetComponent<HealthComponent>(enemy);
                    health.currentHP -= projectile.damage;
                    std::cout << "Projectile " << entity << " hit Enemy " << enemy << "! HP: " << health.currentHP << std::endl;
                    bulletsToDestroy.insert(entity);
                    break; 
                }
            }

           
            if (glm::length(transform.position) > 50.0f) {
                bulletsToDestroy.insert(entity);
            }
        }

        for (auto const& bullet : bulletsToDestroy) {
            registry->DestroyEntity(bullet);
        }
    }
};