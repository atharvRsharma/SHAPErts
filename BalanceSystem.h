#pragma once


#include <algorithm> 
#include <iostream>

class BalanceSystem : public ecs::System {
public:
    // Balance is 0.0 (max shadow) to 1.0 (max light)
    float m_Balance = 0.5f;

    void Init() {
        m_Balance = 0.5f;
    }

    void OnEnemyKilled() {
        m_Balance += 0.05f;
        m_Balance = std::clamp(m_Balance, 0.0f, 1.0f);
        std::cout << "LIGHT energy increased: " << (m_Balance * 100) << "%" << std::endl;
    }

    void OnBuildingDestroyed() {
        m_Balance -= 0.1f;
        m_Balance = std::clamp(m_Balance, 0.0f, 1.0f);
        std::cout << "SHADOW energy increased: " << (m_Balance * 100) << "%" << std::endl;
    }

    float GetBalance() const {
        return m_Balance;
    }

    // at 0.0 (max shadow) -> returns 0.5x
    // at 0.5 (neutral)    -> returns 1.0x
    // at 1.0 (max light)  -> returns 1.5x
    float GetResourceModifier() const {
        return 0.5f + m_Balance;
    }
};