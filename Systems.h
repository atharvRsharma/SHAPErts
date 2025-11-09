// All elements are generic placeholders for testing purposes.
#pragma once

/*
 * This file just contains the declarations for systems
 * that are NOT YET implemented.
 *
 * RenderSystem, UISystem, and InputSystem are defined
 * in their own headers.
 */

#include "ECS.h"
#include <glm/glm.hpp>

 // --- Future Systems (Declarations only) ---

 // Updates entity positions based on MovementComponent
class MovementSystem : public ecs::System {
public:
    void Update(float dt);
};

// Handles combat logic
class CombatSystem : public ecs::System {
public:
    void Update(float dt);
};

// Handles resource generation and global state
class ResourceSystem : public ecs::System {
public:
    void Update(float dt);
};

// Tracks and applies energy balance buffs/debuffs
class EnergyBalanceSystem : public ecs::System {
public:
    void Update(float dt);
};