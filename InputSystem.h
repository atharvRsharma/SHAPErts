#pragma once

#include "ECS.h"
#include "Components.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <optional> 
#include "imgui.h" 

class Game;
class GridSystem;

enum class InputMode {
    SELECT,
    BUILD
};

class InputSystem : public ecs::System {
public:
    GLFWwindow* m_Window;

    void Init(GLFWwindow* window, ecs::Registry* registry, Game* game) {
        m_Window = window;
        m_Game = game;
    }

    void SetWindowSize(int width, int height);
    void UpdateMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition);
    void Update();

    glm::ivec2 GetSelectedGridCoords() const;

    void EnterBuildMode(BuildingType buildType, MeshType meshType, double cost, glm::ivec2 footprint);
    void ExitBuildMode();
    bool IsInBuildMode() const { return m_CurrentMode == InputMode::BUILD; }

    void RotateBuildFootprint(int direction);

private:
    Game* m_Game;
    glm::mat4 m_Projection, m_View, m_InvProjection, m_InvView;
    glm::vec3 m_CameraPosition;
    int m_Width = 0, m_Height = 0;
    bool m_MousePressedLastFrame = false;
    int m_SelectedGridX = -1, m_SelectedGridY = -1;

    InputMode m_CurrentMode = InputMode::SELECT;
    MeshType m_BuildMeshType = MeshType::None;
    BuildingType m_BuildBuildingType = BuildingType::None;
    double m_BuildCost = 0.0;
    glm::ivec2 m_BaseFootprint = { 1, 1 };
    glm::ivec2 m_BuildFootprint = { 1, 1 };

    // --- FIX: This is now 0, 1, 2, or 3 ---
    int m_BuildRotation = 0;
    bool m_LastPlacementValid = false;

    glm::vec3 ScreenToWorldRay(double xpos, double ypos);
    std::optional<glm::vec3> IntersectRayWithPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDirection);
    std::optional<ecs::Entity> GetHighlighter();
    void UpdateHighlighter();
    void HandleMouseClick();
};