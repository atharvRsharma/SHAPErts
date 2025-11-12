// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include <iostream>

class ResourceSystem : public ecs::System {
public:
    void Init(double startingResources = 1000.0) {
        m_CurrentResources = startingResources;
    }

    void Update(float dt) {
        // Later, we'll find all ResourceGenerator components
        // and add to this value.
        // For now, it just holds the value.
    }

    double GetResources() const {
        return m_CurrentResources;
    }

    bool SpendResources(double amount) {
        if (m_CurrentResources >= amount) {
            m_CurrentResources -= amount;
            std::cout << "Spent " << amount << ", " << m_CurrentResources << " remaining." << std::endl;
            return true; // Success
        }
        return false; // Not enough resources
    }

    void AddResources(double amount) {
        m_CurrentResources += amount;
    }

private:
    double m_CurrentResources = 0.0;
};