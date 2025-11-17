#pragma once


#include "ECS.h"
#include "Components.h"
#include "BalanceSystem.h" 
#include <iostream>

class ResourceSystem : public ecs::System {
public:
    void Init(BalanceSystem* balanceSystem, double startingResources = 1000.0) {
        m_BalanceSystem = balanceSystem;
        m_CurrentResources = startingResources;
    }

    void Update(float dt) {
        float modifier = m_BalanceSystem->GetResourceModifier();

        for (auto const& entity : m_Entities) {
            auto& generator = m_Registry->GetComponent<ResourceGeneratorComponent>(entity);
            m_CurrentResources += generator.resourcesPerSecond * modifier * dt;
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

    void AddResources(double amount) {
        m_CurrentResources += amount;
    }

private:
    double m_CurrentResources = 0.0;
    BalanceSystem* m_BalanceSystem; 
};