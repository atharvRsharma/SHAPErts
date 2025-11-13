#include "InputSystem.h"
#include "GridSystem.h"
#include "ResourceSystem.h"
#include "Game.h" 
#include <iostream>
#include <algorithm> // For std::swap

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
    // If any UI window is hovered, we don't check for clicks or update highlighter
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        m_MousePressedLastFrame = false; // Prevent click carry-over
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

void InputSystem::EnterBuildMode(BuildingType buildType, MeshType meshType, double cost, glm::ivec2 footprint) {
    m_CurrentMode = InputMode::BUILD;
    m_BuildBuildingType = buildType;
    m_BuildMeshType = meshType;
    m_BuildCost = cost;
    m_BaseFootprint = footprint;
    m_BuildFootprint = footprint;
    m_BuildRotation = 0;

    auto highlighter = GetHighlighter();
    if (highlighter) {
        m_Registry->GetComponent<MeshComponent>(*highlighter).type = m_BuildMeshType;
    }
    std::cout << "Entering build mode." << std::endl;
}

void InputSystem::ExitBuildMode() {
    m_CurrentMode = InputMode::SELECT;
    m_BuildCost = 0.0;
    m_BuildBuildingType = BuildingType::None;
    m_SelectedGridX = -1;
    m_SelectedGridY = -1;

    auto highlighter = GetHighlighter();
    if (highlighter) {
        auto& render = m_Registry->GetComponent<RenderComponent>(*highlighter);
        auto& transform = m_Registry->GetComponent<TransformComponent>(*highlighter);
        auto& mesh = m_Registry->GetComponent<MeshComponent>(*highlighter);

        render.color.a = 0.0f; // Invisible
        transform.scale = { 1,1,1 };
        transform.rotation = { 0,0,0 };
        mesh.type = MeshType::None; // <-- Hide the mesh
    }
    std::cout << "Exiting build mode." << std::endl;
}

// --- THIS IS THE 4-WAY ROTATION LOGIC ---
void InputSystem::RotateBuildFootprint(int direction) {
    if (direction == 0) return;
    int delta = (direction > 0) ? 1 : -1; // +1 for scroll up, -1 for scroll down

    // support 4 orientations: 0, 1, 2, 3 (0, 90, 180, 270 deg)
    m_BuildRotation = (m_BuildRotation + delta + 4) % 4;

    // For rotations 0 and 2 (0, 180 deg), use base footprint
    // For rotations 1 and 3 (90, 270 deg), use swapped footprint
    if ((m_BuildRotation % 2) == 0) {
        m_BuildFootprint = m_BaseFootprint;
    }
    else {
        m_BuildFootprint = { m_BaseFootprint.y, m_BaseFootprint.x };
    }
}
// --- END OF FIX ---

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
    auto& mesh = m_Registry->GetComponent<MeshComponent>(highlighter);

    mesh.type = m_BuildMeshType;

    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);
    glm::vec3 rayDir = ScreenToWorldRay(xpos, ypos);
    std::optional<glm::vec3> intersection = IntersectRayWithPlane(m_CameraPosition, rayDir);

    if (intersection) {
        glm::ivec2 anchorGridPos = gridSystem->WorldToGrid(*intersection);

        m_LastPlacementValid = true;
        for (int x = 0; x < m_BuildFootprint.x; ++x) {
            for (int z = 0; z < m_BuildFootprint.y; ++z) {
                int currentX = anchorGridPos.x + x;
                int currentZ = anchorGridPos.y + z;

                if (!gridSystem->IsValidTile(currentX, currentZ) || gridSystem->IsTileOccupied(currentX, currentZ)) {
                    m_LastPlacementValid = false;
                    break;
                }
            }
            if (!m_LastPlacementValid) break;
        }

        // --- THIS IS THE ROTATION FIX ---
        // Scale is based on the *footprint*
        transform.scale = glm::vec3(m_BuildFootprint.x, 1.0f, m_BuildFootprint.y);
        // Rotation is based on the *logical rotation value*
        float angleDegrees = static_cast<float>(m_BuildRotation) * 90.0f;
        transform.rotation = { 0.0f, angleDegrees, 0.0f };
        // --- END OF FIX ---

        glm::vec3 worldPos = gridSystem->GridToWorld(anchorGridPos.x, anchorGridPos.y);

        float yOffset = 0.01f;
        if (m_BuildMeshType == MeshType::Cube || m_BuildMeshType == MeshType::Turret || m_BuildMeshType == MeshType::Sphere) {
            yOffset = 0.5f;
        }

        transform.position = {
            worldPos.x + (m_BuildFootprint.x / 2.0f) - 0.5f,
            yOffset,
            worldPos.z + (m_BuildFootprint.y / 2.0f) - 0.5f
        };

        render.color = m_LastPlacementValid ?
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) : // Green
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
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

                glm::ivec2 anchorGridPos = gridSystem->WorldToGrid(h_transform.position -
                    glm::vec3((m_BuildFootprint.x / 2.0f) - 0.5f, 0.0f, (m_BuildFootprint.y / 2.0f) - 0.5f));

                auto building = m_Registry->CreateEntity();

                float yOffset = 0.0f;
                if (m_BuildMeshType == MeshType::Cube || m_BuildMeshType == MeshType::Turret || m_BuildMeshType == MeshType::Sphere) {
                    yOffset = 0.5f;
                }
                glm::vec3 buildPos = { h_transform.position.x, yOffset, h_transform.position.z };

                m_Registry->AddComponent(building, TransformComponent{
                    buildPos,
                    h_transform.scale,
                    h_transform.rotation // <-- This now correctly copies the {0, 90, 180, 270} rotation
                    });

                glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
                if (m_BuildBuildingType == BuildingType::ResourceNode) {
                    color = { 0.2f, 0.8f, 0.2f, 1.0f }; // Green
                }
                m_Registry->AddComponent(building, RenderComponent{ color });

                m_Registry->AddComponent(building, MeshComponent{ m_BuildMeshType });
                m_Registry->AddComponent(building, BuildingComponent{ m_BuildBuildingType });

                if (m_BuildBuildingType == BuildingType::ResourceNode) {
                    m_Registry->AddComponent(building, ResourceGeneratorComponent{});
                }

                if (m_BuildBuildingType == BuildingType::Base) {
                    m_Game->OnBasePlaced(buildPos);
                }

                for (int x = 0; x < m_BuildFootprint.x; ++x) {
                    for (int z = 0; z < m_BuildFootprint.y; ++z) {
                        gridSystem->SetTileOccupied(anchorGridPos.x + x, anchorGridPos.y + z, true);
                    }
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
        auto& h_mesh = m_Registry->GetComponent<MeshComponent>(*highlighter);

        h_transform.scale = { 1,1,1 };
        h_transform.rotation = { 0,0,0 };
        h_mesh.type = MeshType::Quad;

        if (intersection) {
            auto gridSystem = m_Registry->GetSystem<GridSystem>();
            glm::ivec2 gridPos = gridSystem->WorldToGrid(*intersection);

            if (gridSystem->IsValidTile(gridPos.x, gridPos.y)) {
                m_SelectedGridX = gridPos.x;
                m_SelectedGridY = gridPos.y;

                glm::vec3 worldPos = gridSystem->GridToWorld(gridPos.x, gridPos.y);
                h_transform.position = { worldPos.x, 0.01f, worldPos.z };
                h_render.color = { 1.0f, 1.0f, 0.0f, 1.0f };
            }
            else {
                h_render.color.a = 0.0f;
            }
        }
        else {
            h_render.color.a = 0.0f;
        }
    }
}