#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h" 
#include "imgui.h"
#include <string>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Game.h"
#include "ResourceSystem.h"

// --- Define build costs ---
const double BASE_COST = 100.0;
const double TURRET_COST = 150.0;
const double NODE_COST = 50.0;

struct UIState {
    double resources = 0.0;
    int unitCount = 0;
    std::string selectedTileInfo = "None";
    float fps = 0.0f;
};

class UISystem : public ecs::System {
public:
    void Init(ecs::Registry* registry) {
        m_State = UIState{};
    }

    void Update(float dt) {
        // --- Calculate FPS ---
        m_FrameCount++;
        m_TimeAccumulator += dt;
        if (m_TimeAccumulator >= 1.0f) {
            m_State.fps = (float)m_FrameCount / m_TimeAccumulator;
            m_TimeAccumulator = 0.0f;
            m_FrameCount = 0;
        }

        // --- Get Selected Tile ---
        auto inputSystem = m_Registry->GetSystem<InputSystem>();
        glm::ivec2 coords = inputSystem->GetSelectedGridCoords();
        if (coords.x != -1) {
            m_State.selectedTileInfo = "Tile (" + std::to_string(coords.x) + ", " + std::to_string(coords.y) + ")";
        }
        else {
            m_State.selectedTileInfo = "None";
        }

        // --- Get Resources ---
        auto resourceSystem = m_Registry->GetSystem<ResourceSystem>();
        m_State.resources = resourceSystem->GetResources();
    }

    void Render(ecs::Registry* registry) {
        DrawMainHUD(registry);
        DrawDebugWindow(registry);
        DrawBuildMenu(registry);
    }

private:
    UIState m_State;
    int m_FrameCount = 0;
    float m_TimeAccumulator = 0.0f;

    // --- MAIN HUD (BACK AGAIN) ---
    void DrawMainHUD(ecs::Registry* registry) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);

        if (ImGui::Begin("Game HUD", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Resources: %d", (int)m_State.resources);
            ImGui::Text("Unit Count: %d", (int)m_State.unitCount);
            ImGui::Text("FPS: %.1f", m_State.fps);
            ImGui::Separator();
            ImGui::Text("Selected: %s", m_State.selectedTileInfo.c_str());
            ImGui::Text("Energy Balance");
            float progress = 0.5f;
            ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "50%");

            ImGui::End();
        }
    }

    // --- DEBUG WINDOW (BACK AGAIN) ---
    void DrawDebugWindow(ecs::Registry* registry) {
        if (ImGui::Begin("Debug Info")) {
            ImGui::Text("Entities (UI System): %zu", m_Entities.size());
            ImGui::Text("Living Entities (Total): %d", registry->GetLivingEntityCount());

            Game* game = static_cast<Game*>(glfwGetWindowUserPointer(m_Registry->GetSystem<InputSystem>()->m_Window));
            if (game) {
                ImGui::Text("God Mode: %s", game->m_IsGodMode ? "ON" : "OFF");
                glm::vec3 camPos = game->m_IsGodMode ? game->m_FlyCamera.Position : game->m_OrbitCamera.GetPosition();
                ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
            }

            if (ImGui::CollapsingHeader("Systems")) {
                ImGui::Text("RenderSystem: %zu entities", m_Registry->GetSystem<RenderSystem>()->m_Entities.size());
                ImGui::Text("UISystem: %zu entities", m_Registry->GetSystem<UISystem>()->m_Entities.size());
                ImGui::Text("InputSystem: %zu entities", m_Registry->GetSystem<InputSystem>()->m_Entities.size());
            }
            ImGui::End();
        }
    }

    // --- BUILD MENU ---
    void DrawBuildMenu(ecs::Registry* registry) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 210, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiCond_Always);

        if (!ImGui::Begin("Build Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
            ImGui::End();
            return;
        }

        auto game = static_cast<Game*>(glfwGetWindowUserPointer(m_Registry->GetSystem<InputSystem>()->m_Window));
        auto inputSystem = m_Registry->GetSystem<InputSystem>();
        auto resourceSystem = m_Registry->GetSystem<ResourceSystem>();
        double currentResources = resourceSystem->GetResources();

        if (inputSystem->IsInBuildMode() &&
            ImGui::GetIO().MouseClicked[0] &&
            !ImGui::IsWindowHovered())
        {
            inputSystem->ExitBuildMode();
        }


        // If we're in build mode, just show a cancel button
        if (inputSystem->IsInBuildMode()) {
            ImGui::Text("Placing building...");
            ImGui::Separator();
            if (ImGui::Button("Cancel")) {
                inputSystem->ExitBuildMode();
            }
            ImGui::End();
            return;
        }

        // --- Category: Base ---
        if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (game->m_BasePlaced) {
                ImGui::Text("Home Base placed.");
            }
            else {
                bool canAfford = currentResources >= BASE_COST;
                if (!canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

                if (ImGui::Button("Home Base [100]", ImVec2(-1, 0)) && canAfford) {
                    inputSystem->EnterBuildMode(MeshType::Pyramid, BuildingType::Base, BASE_COST);
                }

                if (!canAfford) ImGui::PopStyleVar();
            }
        }

        // --- Category: Defense ---
        if (ImGui::CollapsingHeader("Defense", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (!game->m_BasePlaced) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                ImGui::Button("Turret [150]", ImVec2(-1, 0));
                ImGui::PopStyleVar();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Place Home Base first!");
            }
            else {
                bool canAfford = currentResources >= TURRET_COST;
                if (!canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

                if (ImGui::Button("Turret [150]", ImVec2(-1, 0)) && canAfford) {
                    inputSystem->EnterBuildMode(MeshType::Turret, BuildingType::Turret, TURRET_COST);
                }

                if (!canAfford) ImGui::PopStyleVar();
            }
        }

        // --- Category: Resources ---
        if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (!game->m_BasePlaced) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                ImGui::Button("Node [50]", ImVec2(-1, 0));
                ImGui::PopStyleVar();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Place Home Base first!");
            }
            else {
                bool canAfford = currentResources >= NODE_COST;
                if (!canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

                if (ImGui::Button("Node [50]", ImVec2(-1, 0)) && canAfford) {
                    inputSystem->EnterBuildMode(MeshType::Cube, BuildingType::ResourceNode, NODE_COST);
                }

                if (!canAfford) ImGui::PopStyleVar();
            }
        }

        ImGui::End();
    }
};