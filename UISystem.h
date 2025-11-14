#pragma once



#include "ECS.h"
#include "Components.h" 
#include "imgui.h"
#include <string>
#include <GLFW/glfw3.h> 

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Game.h"
#include "ResourceSystem.h"
#include "BalanceSystem.h"


const double BASE_COST = 100.0;
const double TURRET_COST = 150.0;
const double NODE_COST = 50.0;
const double BOMB_COST = 70.0;




struct UIState {
    double resources = 0.0;
    int unitCount = 0;
    std::string selectedTileInfo = "None";
    float balance = 0.5f;
    double fps = 0.0;
};

class UISystem : public ecs::System {
public:
    void Init(ecs::Registry* registry) {
        m_State = UIState{};
    }

    void Update(float dt) {
        
        

        
        auto inputSystem = m_Registry->GetSystem<InputSystem>();
        glm::ivec2 coords = inputSystem->GetSelectedGridCoords();
        if (coords.x != -1) {
            m_State.selectedTileInfo = "Tile (" + std::to_string(coords.x) + ", " + std::to_string(coords.y) + ")";
        }
        else {
            m_State.selectedTileInfo = "None";
        }

        
        auto resourceSystem = m_Registry->GetSystem<ResourceSystem>();
        m_State.resources = resourceSystem->GetResources();

        auto balanceSystem = m_Registry->GetSystem<BalanceSystem>();
        m_State.balance = balanceSystem->GetBalance();
    }

    void Render(ecs::Registry* registry) {

        static double lastTime = glfwGetTime();
        static int frames = 0;
        double currentTime = glfwGetTime();
        frames++;
        if (currentTime - lastTime >= 1.0) {
            m_State.fps = (double)frames / (currentTime - lastTime);
            frames = 0;
            lastTime = currentTime;
        }
        DrawMainHUD(registry);
        DrawDebugWindow(registry);
        DrawBuildMenu(registry);
        
    }

private:
    UIState m_State;


    void DrawMainHUD(ecs::Registry* registry) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);

        auto balanceSystem = m_Registry->GetSystem<BalanceSystem>();

        if (ImGui::Begin("Game HUD", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::Text("Resources: %d", (int)m_State.resources);
            ImGui::Text("Unit Count: %d", (int)m_State.unitCount);
            ImGui::Text("FPS: %.1f", m_State.fps);
            ImGui::Separator();
            ImGui::Text("Selected: %s", m_State.selectedTileInfo.c_str());
            ImGui::Text("Energy Balance");

           
            std::string text = std::to_string((int)m_State.balance * 100) + "%";
            ImGui::ProgressBar(m_State.balance, ImVec2(-1.0f, 0.0f), text.c_str());

            ImGui::End();
        }
    }

    void DrawDebugWindow(ecs::Registry* registry) {
        if (ImGui::Begin("Debug Info")) {
            ImGui::Text("Entities (UI System): %zu", m_Entities.size());
            ImGui::Text("Living Entities (Total): %d", registry->GetLivingEntityCount());

            Game* game = static_cast<Game*>(glfwGetWindowUserPointer(m_Registry->GetSystem<InputSystem>()->m_Window));
            if (game) {
                ImGui::Text("God Mode: %s", game->m_IsGodMode ? "ON" : "OFF");
                glm::vec3 camPos = game->m_IsGodMode ? game->m_FlyCamera.Position : game->m_OrbitCamera.GetPosition();
                ImGui::Text("Camera Pos: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);

                ImGui::Separator();
                if (ImGui::Button("Spawn Enemy")) {
                    game->SpawnEnemyAt({ 9.5f, 0.5f, 9.5f });
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Spawns 1 enemy at (10, 10)");
            }

            if (ImGui::CollapsingHeader("Systems")) {
                ImGui::Text("RenderSystem: %zu entities", m_Registry->GetSystem<RenderSystem>()->m_Entities.size());
                ImGui::Text("UISystem: %zu entities", m_Registry->GetSystem<UISystem>()->m_Entities.size());
                ImGui::Text("InputSystem: %zu entities", m_Registry->GetSystem<InputSystem>()->m_Entities.size());

            }
            
        }
        ImGui::End();
    }


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


        if (inputSystem->IsInBuildMode()) {
            ImGui::Text("Placing building...");
            ImGui::Text("Scroll wheel to rotate.");
            ImGui::Separator();
            if (ImGui::Button("Cancel")) {
                inputSystem->ExitBuildMode();
            }
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (game->m_BasePlaced) {
                ImGui::Text("Home Base placed.");
            }
            else {
                bool canAfford = currentResources >= BASE_COST;
                if (!canAfford) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

                if (ImGui::Button("Home Base [100]", ImVec2(-1, 0)) && canAfford) {
                    inputSystem->EnterBuildMode(BuildingType::Base, MeshType::Pyramid, BASE_COST, { 2, 2 });
                }

                if (!canAfford) ImGui::PopStyleVar();
            }
        }

        if (ImGui::CollapsingHeader("defense", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (!game->m_BasePlaced) {
                ImGui::BeginDisabled(); // disable all children until EndDisabled
                ImGui::Button("Turret [150]", ImVec2(-1, 0));
                ImGui::Button("Bomb [70]", ImVec2(-1, 0));
                ImGui::EndDisabled();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("home base has to be placed first");
            }
            else {
                bool canAffordTurret = currentResources >= TURRET_COST;
                bool canAffordBomb = currentResources >= BOMB_COST;

                //disable turret button when unaffordable
                if (!canAffordTurret) ImGui::BeginDisabled();
                if (ImGui::Button("Turret [150]", ImVec2(-1, 0)) && canAffordTurret) {
                    inputSystem->EnterBuildMode(BuildingType::Turret, MeshType::Turret, TURRET_COST, { 1, 1 });
                }
                if (!canAffordTurret) ImGui::EndDisabled();

                //disable bomb button when unaffordable
                if (!canAffordBomb) ImGui::BeginDisabled();
                if (ImGui::Button("Bomb [70]", ImVec2(-1, 0)) && canAffordBomb) {
                    inputSystem->EnterBuildMode(BuildingType::Bomb, MeshType::Sphere, BOMB_COST, { 1, 1 });
                }
                if (!canAffordBomb) ImGui::EndDisabled();
            }
        }


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
                    inputSystem->EnterBuildMode(BuildingType::ResourceNode, MeshType::Cube, NODE_COST, { 1, 1 });
                }

                if (!canAfford) ImGui::PopStyleVar();
            }
        }

        ImGui::End();
    }

};