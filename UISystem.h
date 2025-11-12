#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h" 
#include "imgui.h"
#include <string>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Game.h" // <-- Need this to get God Mode state

struct GameState {
    float resources = 1000.0f;
    int unitCount = 0;
    std::string selectedTileInfo = "None";
    float fps = 0.0f;
};

class UISystem : public ecs::System {
public:
    void Init(ecs::Registry* registry) {
        m_State = GameState{};
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
    }

    void Render(ecs::Registry* registry) {
        DrawMainHUD(registry);
        DrawDebugWindow(registry);
    }

private:
    GameState m_State;
    int m_FrameCount = 0;
    float m_TimeAccumulator = 0.0f;

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

    void DrawDebugWindow(ecs::Registry* registry) {
        if (ImGui::Begin("Debug Info")) {
            ImGui::Text("Entities (UI System): %zu", m_Entities.size());
            ImGui::Text("Living Entities (Total): %d", registry->GetLivingEntityCount());

            // --- NEW: Get Game to read camera state ---
            // This is a safe way to get the Game pointer
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
           
        }
        ImGui::End();
    }
};