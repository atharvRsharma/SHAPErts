// All elements are generic placeholders for testing purposes.
#pragma once


#include "ECS.h" 
#include "Components.h"
#include <iostream>

class ResourceSystem : public ecs::System {
public:
    void Init(double startingResources = 1000.0) {
        m_CurrentResources = startingResources;
    }

    void Update(float dt) {
        for (auto const& entity : m_Entities) {
            auto& generator = m_Registry->GetComponent<ResourceGeneratorComponent>(entity);
            m_CurrentResources += generator.resourcesPerSecond * dt;
        }
    }

    double GetResources() const {
        return m_CurrentResources;
    }

    bool SpendResources(double amount) {
        if (m_CurrentResources >= amount) {
            m_CurrentResources -= amount;
            std::cout << "Spent " << amount << ", " << m_CurrentResources << " remaining." << std::endl;
            return true; 
        }
        return false; 
    }


private:
    double m_CurrentResources = 0.0;
};