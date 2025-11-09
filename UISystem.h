// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h" // Include components to read their data
#include "imgui.h"
#include <string>

#include "RenderSystem.h"
#include "InputSystem.h"

// We need to forward-declare the other systems to get their info
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
        // Nothing special to init for ImGui itself,
        // but we can set up our internal state
        m_State = GameState{};
    }

    void Update(float dt) {
        // Update is called in the fixed simulation step.
        // We can use it to gather data for the next render.

        // Calculate FPS
        m_FrameCount++;
        m_TimeAccumulator += dt;
        if (m_TimeAccumulator >= 1.0f) {
            m_State.fps = (float)m_FrameCount / m_TimeAccumulator;
            m_TimeAccumulator = 0.0f;
            m_FrameCount = 0;
        }

        // Find the selected tile and update UI state
        m_State.selectedTileInfo = "None"; // Default
        // m_Entities is populated by the ECS with entities that match our signature
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
        // This is called in the render step.
        // We just draw ImGui windows.

        DrawMainHUD(registry);
        DrawDebugWindow(registry);
    }

private:
    GameState m_State;

    // For FPS calculation
    int m_FrameCount = 0;
    float m_TimeAccumulator = 0.0f;

    void DrawMainHUD(ecs::Registry* registry) {
        // Set a window position and size
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

                // --- THIS IS THE FIX ---
                // We must include the headers for these systems at the top
                // of this file, or forward-declare them.
                ImGui::Text("RenderSystem: %zu entities", m_Registry->GetSystem<RenderSystem>()->m_Entities.size());
                ImGui::Text("UISystem: %zu entities", m_Registry->GetSystem<UISystem>()->m_Entities.size());
                ImGui::Text("InputSystem: %zu entities", m_Registry->GetSystem<InputSystem>()->m_Entities.size());
                // --- END OF FIX ---

            }
            ImGui::End();
        }
    }
};