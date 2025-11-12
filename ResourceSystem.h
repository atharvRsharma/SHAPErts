// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"

class ResourceSystem : public ecs::System {
public:
    void Init() {
        m_CurrentResources = 1000.0;
    }

    // We'll call this from the main game loop
    void Update(float dt) {
        // TODO: Add resource generation logic
        // m_CurrentResources += m_IncomePerSecond * dt;
    }

    double GetResources() const {
        return m_CurrentResources;
    }

    bool SpendResources(double amount) {
        if (m_CurrentResources >= amount) {
            m_CurrentResources -= amount;
            return true; // Success
        }
        return false; // Not enough resources
    }

private:
    double m_CurrentResources = 0.0;
};