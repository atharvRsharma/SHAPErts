#include "InputSystem.h"
#include "GridSystem.h"
#include "ResourceSystem.h" 
#include "Game.h" 
#include <iostream>

// --- Definitions for the functions in InputSystem.h ---

void InputSystem::SetWindowSize(int width, int height) {
    m_Width = width;
    m_Height = height;
}

void InputSystem::UpdateMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition) {
    m_Projection = projection;
    m_View = view;
    m_InvProjection = glm::inverse(projection);
    m_InvView = glm::inverse(view);
    m_CameraPosition = cameraPosition;
}

void InputSystem::Update() {
    if (ImGui::GetIO().WantCaptureMouse) {
        m_MousePressedLastFrame = false;
        return;
    }

    if (m_CurrentMode == InputMode::BUILD) {
        UpdateHighlighter();
    }

    bool mousePressed = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (mousePressed && !m_MousePressedLastFrame) {
        HandleMouseClick();
    }

    m_MousePressedLastFrame = mousePressed;
}

glm::ivec2 InputSystem::GetSelectedGridCoords() const {
    return { m_SelectedGridX, m_SelectedGridY };
}

void InputSystem::EnterBuildMode(MeshType meshType, BuildingType buildType, double cost) {
    m_CurrentMode = InputMode::BUILD;
    m_BuildMeshType = meshType;
    m_BuildBuildingType = buildType;
    m_BuildCost = cost;
    std::cout << "Entering build mode." << std::endl;
}

void InputSystem::ExitBuildMode() {
    m_CurrentMode = InputMode::SELECT;
    m_BuildCost = 0.0;
    m_BuildMeshType = MeshType::None;
    m_BuildBuildingType = BuildingType::None;
    m_SelectedGridX = -1;
    m_SelectedGridY = -1;

    auto highlighter = GetHighlighter();
    if (highlighter) {
        auto& render = m_Registry->GetComponent<RenderComponent>(*highlighter);
        render.color.a = 0.0f; // Invisible
    }
    std::cout << "Exiting build mode." << std::endl;
}

std::optional<ecs::Entity> InputSystem::GetHighlighter() {
    for (auto const& entity : m_Entities) {
        return entity;
    }
    return std::nullopt;
}

std::optional<glm::vec3> InputSystem::IntersectRayWithPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) {
    glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
    glm::vec3 planePoint(0.0f, 0.0f, 0.0f);

    float denom = glm::dot(planeNormal, rayDirection);
    if (std::abs(denom) > 1e-6) {
        float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
        if (t >= 0) {
            return rayOrigin + rayDirection * t;
        }
    }
    return std::nullopt;
}

glm::vec3 InputSystem::ScreenToWorldRay(double xpos, double ypos) {
    float ndcX = (2.0f * (float)xpos) / (float)m_Width - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)ypos) / (float)m_Height;

    glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 eyeCoords = m_InvProjection * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
    glm::vec4 worldRay = m_InvView * eyeCoords;

    return glm::normalize(glm::vec3(worldRay));
}

void InputSystem::UpdateHighlighter() {
    auto highlighterOpt = GetHighlighter();
    if (!highlighterOpt) return;
    ecs::Entity highlighter = *highlighterOpt;

    auto gridSystem = m_Registry->GetSystem<GridSystem>();
    auto& transform = m_Registry->GetComponent<TransformComponent>(highlighter);
    auto& render = m_Registry->GetComponent<RenderComponent>(highlighter);

    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    glm::vec3 rayDir = ScreenToWorldRay(xpos, ypos);
    std::optional<glm::vec3> intersection = IntersectRayWithPlane(m_CameraPosition, rayDir);

    if (intersection) {
        glm::ivec2 gridPos = gridSystem->WorldToGrid(*intersection);
        m_LastPlacementValid = gridSystem->IsValidTile(gridPos.x, gridPos.y) &&
            !gridSystem->IsTileOccupied(gridPos.x, gridPos.y);

        glm::vec3 worldPos = gridSystem->GridToWorld(gridPos.x, gridPos.y);
        transform.position = { worldPos.x, 0.01f, worldPos.z };
        render.color = m_LastPlacementValid ?
            glm::vec4(0.0f, 1.0f, 0.0f, 0.5f) :
            glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
    }
    else {
        render.color.a = 0.0f;
        m_LastPlacementValid = false;
    }
}

void InputSystem::HandleMouseClick() {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    if (m_CurrentMode == InputMode::BUILD) {
        if (m_LastPlacementValid) {
            auto gridSystem = m_Registry->GetSystem<GridSystem>();
            auto resourceSystem = m_Registry->GetSystem<ResourceSystem>();

            if (resourceSystem->SpendResources(m_BuildCost)) {
                auto highlighter = GetHighlighter();
                auto& h_transform = m_Registry->GetComponent<TransformComponent>(*highlighter);
                glm::ivec2 gridPos = gridSystem->WorldToGrid(h_transform.position);

                auto building = m_Registry->CreateEntity();
                m_Registry->AddComponent(building, TransformComponent{
                    {h_transform.position.x, 0.0f, h_transform.position.z},
                    {1.0f, 1.0f, 1.0f},
                    {0.0f, 0.0f, 0.0f}
                    });
                m_Registry->AddComponent(building, RenderComponent{ {1.0f, 1.0f, 1.0f, 1.0f} });
                m_Registry->AddComponent(building, MeshComponent{ m_BuildMeshType });
                m_Registry->AddComponent(building, BuildingComponent{ m_BuildBuildingType });

                gridSystem->SetTileOccupied(gridPos.x, gridPos.y, true);

                if (m_BuildBuildingType == BuildingType::Base) {
                    m_Game->OnBasePlaced();
                }
                ExitBuildMode();
            }
            else {
                std::cout << "Not enough resources!" << std::endl;
            }
        }

    }
    else if (m_CurrentMode == InputMode::SELECT) {
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);
        glm::vec3 rayDir = ScreenToWorldRay(xpos, ypos);
        std::optional<glm::vec3> intersection = IntersectRayWithPlane(m_CameraPosition, rayDir);

        m_SelectedGridX = -1;
        m_SelectedGridY = -1;
        auto highlighter = GetHighlighter();
        if (!highlighter) return;

        auto& h_transform = m_Registry->GetComponent<TransformComponent>(*highlighter);
        auto& h_render = m_Registry->GetComponent<RenderComponent>(*highlighter);

        if (intersection) {
            auto gridSystem = m_Registry->GetSystem<GridSystem>();
            glm::ivec2 gridPos = gridSystem->WorldToGrid(*intersection);

            if (gridSystem->IsValidTile(gridPos.x, gridPos.y)) {
                m_SelectedGridX = gridPos.x;
                m_SelectedGridY = gridPos.y;

                glm::vec3 worldPos = gridSystem->GridToWorld(gridPos.x, gridPos.y);
                h_transform.position = { worldPos.x, 0.01f, worldPos.z };
                h_render.color = { 1.0f, 1.0f, 0.0f, 0.5f };
            }
            else {
                h_render.color.a = 0.0f; // Hide if off-grid
            }
        }
        else {
            h_render.color.a = 0.0f; // Hide if off-plane
        }
    }
}