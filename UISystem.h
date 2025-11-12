// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h" // Include components to read their data
#include "imgui.h"
#include <string>

#include "RenderSystem.h"
#include "InputSystem.h"

class RenderSystem;
class InputSystem;

// A simple struct to hold global game state for the UI
struct GameState {
    float resources = 1000.0f;
    int unitCount = 0;
    float lightEnergy = 0.5f; // Range 0.0 to 1.0
    float shadowEnergy = 0.5f; // Range 0.0 to 1.0
    std::string selectedTileInfo = "None";
    float fps = 0.0f;
};


class UISystem : public ecs::System {
public:
    void Init(ecs::Registry* registry) {
        m_State = GameState{};
    }

    void Update(float dt) {
        static double lastTime = glfwGetTime();
        static int frames = 0;
        double currentTime = glfwGetTime();
        frames++;


        if (currentTime - lastTime >= 1.0) { 
            m_State.fps = (float)frames / (currentTime - lastTime);
            frames = 0;
            lastTime = currentTime;
        }


        m_State.selectedTileInfo = "None"; 
        for (auto const& entity : m_Entities) {
            auto& selectable = m_Registry->GetComponent<SelectableComponent>(entity);
            if (selectable.isSelected) {
                auto& tile = m_Registry->GetComponent<GridTileComponent>(entity);
                m_State.selectedTileInfo = "Tile (" + std::to_string(tile.x) + ", " + std::to_string(tile.y) + ")";
                break; // Found it, stop searching
            }
        }

        // TODO: Update other state variables
        // m_State.resources = ...
        // m_State.unitCount = ...
        // m_State.lightEnergy = ...
        // m_State.shadowEnergy = ...
    }

    void Render(ecs::Registry* registry) {
        DrawMainHUD(registry);
        DrawDebugWindow(registry);
    }

private:
    GameState m_State;


    void DrawMainHUD(ecs::Registry* registry) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);

        if (ImGui::Begin("Game HUD", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Resources: %d", (int)m_State.resources);
            ImGui::Text("Unit Count: %d", (int)m_State.unitCount);
            ImGui::Text("FPS: %.1f", m_State.fps);

            ImGui::Separator();
            ImGui::Text("Selected: %s", m_State.selectedTileInfo.c_str());

            // --- Energy Meter ---
            ImGui::Text("Energy Balance");
            // Calculate total balance from -1.0 (max shadow) to +1.0 (max light)
            float balance = m_State.lightEnergy - m_State.shadowEnergy;
            // Map to 0.0 - 1.0 for the progress bar
            float progress = (balance + 1.0f) / 2.0f;

            // Custom colored progress bar
            ImVec4 barColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f); // Blue
            if (balance < 0) {
                barColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
            }
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
            ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
            ImGui::PopStyleColor();

            ImGui::End();
        }
    }

    void DrawDebugWindow(ecs::Registry* registry) {
        if (ImGui::Begin("Debug Info")) {
            ImGui::Text("Entities (UI System): %zu", m_Entities.size());
            ImGui::Text("Living Entities (Total): %d", registry->GetLivingEntityCount());

            if (ImGui::CollapsingHeader("Systems")) {
                ImGui::Text("RenderSystem: %zu entities", m_Registry->GetSystem<RenderSystem>()->m_Entities.size());
                ImGui::Text("UISystem: %zu entities", m_Registry->GetSystem<UISystem>()->m_Entities.size());
                ImGui::Text("InputSystem: %zu entities", m_Registry->GetSystem<InputSystem>()->m_Entities.size());


            }
            
        }
        ImGui::End();
    }
};