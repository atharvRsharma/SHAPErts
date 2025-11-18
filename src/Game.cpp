#include "Game.h"
#include "ECS.h"
#include "Components.h"
#include "RenderSystem.h"
#include "UISystem.h"
#include "InputSystem.h"
#include "ResourceSystem.h"
#include "GridSystem.h"
#include "MovementSystem.h" 
#include "EnemyAISystem.h"  
#include "CombatSystem.h" 
#include "BalanceSystem.h"
#include "ProjectileSystem.h"
#include "CollisionSystem.h"
#include "Serializer.h"

#include <stb/stb_image.h>

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <iostream>
//#include <fstream>     
//#include <nlohmann/json.hpp>
//using json = nlohmann::json;

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"



void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) {
        return;
    }

    glViewport(0, 0, width, height);
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->m_Width = width;
        game->m_Height = height;
        game->m_InputSystem->SetWindowSize(width, height);
    }
}

void Game::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (ImGui::GetIO().WantCaptureMouse || !game) return;

    if (game->m_IsGodMode) return;

    if (game->m_CurrentState != AppState::PLAYING) return;

    if (game->m_InputSystem->IsInBuildMode()) {
        game->m_InputSystem->RotateBuildFootprint(yoffset > 0 ? 1 : -1);
    }
    else {
        game->m_OrbitCamera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void Game::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    if (ImGui::GetIO().WantCaptureMouse) return;
    if (game->m_IsGodMode) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            game->m_IsOrbiting = true;
            glfwGetCursorPos(window, &game->m_LastMouseX, &game->m_LastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            game->m_IsOrbiting = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            game->m_IsPanning = true;
            glfwGetCursorPos(window, &game->m_LastMouseX, &game->m_LastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            game->m_IsPanning = false;
        }
    }

    if (game->m_IsGodMode || game->m_CurrentState != AppState::PLAYING) return;
}

void Game::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    if (game->m_CurrentState == AppState::PAUSED && !game->m_IsGodMode) {
        return;
    }

    float xoffset = static_cast<float>(xpos - game->m_LastMouseX);
    float yoffset = static_cast<float>(game->m_LastMouseY - ypos);

    if (game->m_IsGodMode || game->m_IsOrbiting || game->m_IsPanning) {
        game->m_LastMouseX = xpos;
        game->m_LastMouseY = ypos;
    }

    if (game->m_CurrentState == AppState::PAUSED) return;

    if (game->m_IsGodMode)
    {
        game->m_FlyCamera.ProcessMouseLook(xoffset, yoffset);
    }
    else if (game->m_IsOrbiting)
    {
        game->m_OrbitCamera.ProcessMouseOrbit(xoffset);
    }
    else if (game->m_IsPanning)
    {
        game->m_OrbitCamera.ProcessMousePan(xoffset, yoffset);
    }
}

void Game::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game || action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        if (game->m_CurrentState == AppState::PLAYING) {
            game->SetAppState(AppState::PAUSED);
        }
        else if (game->m_CurrentState == AppState::PAUSED) {
            game->SetAppState(AppState::PLAYING);
        }
        return;
    }

    if (game->m_CurrentState != AppState::PLAYING) return;

    game->m_CheatCodeBuffer.push_back(key);
    game->m_ToggleFullscreenBuffer.push_back(key);

    if (game->m_CheatCodeBuffer.size() > game->m_GodModeCode.size()) {
        game->m_CheatCodeBuffer.erase(game->m_CheatCodeBuffer.begin());
    }
    if (game->m_CheatCodeBuffer == game->m_GodModeCode) {
        game->ToggleGodMode();
    }

    if (game->m_ToggleFullscreenBuffer.size() > game->m_ToggleFullscreenCode.size()) {
        game->m_ToggleFullscreenBuffer.erase(game->m_ToggleFullscreenBuffer.begin());
    }
    if (game->m_ToggleFullscreenBuffer == game->m_ToggleFullscreenCode) {
        game->SetWindowMode(!game->m_IsBorderless);
    }

}



Game::Game(int width, int height, const std::string& title)
    : m_Window(nullptr), m_Width(width), m_Height(height), m_Title(title),
    m_OrbitCamera(glm::vec3(0.0f)),
    m_FlyCamera(glm::vec3(0.0f, 15.0f, 15.0f)),
    m_CurrentState(AppState::MAIN_MENU),
    m_WindowedWidth(width), m_WindowedHeight(height)
{
    Init();
}

Game::~Game() { Cleanup(); }


void Game::SetAppState(AppState newState) {
    m_CurrentState = newState;

    if (m_CurrentState == AppState::PLAYING) {
        if (!m_IsGodMode) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Game::SetWindowMode(bool fullscreen) {
    m_IsBorderless = fullscreen;
    m_ToggleFullscreenBuffer.clear();
    if (fullscreen) {
        //set to borderless fullscr
        glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowPos(m_Window, 0, 0);
        glfwSetWindowSize(m_Window, m_PrimaryMode->width, m_PrimaryMode->height);
    }
    else {
        //... windowed mode
        glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_TRUE);
        glfwSetWindowPos(m_Window, 100, 100);
        glfwSetWindowSize(m_Window, m_WindowedWidth, m_WindowedHeight);
    }
}

void Game::OnBasePlaced(glm::vec3 position) {
    m_BasePlaced = true;
    m_BasePosition = position;
    std::cout << "Base placed at: " << position.x << "," << position.y << "," << position.z << std::endl;
}

void Game::SpawnEnemyAt(glm::vec3 position) {
    if (!m_BasePlaced) {
        std::cout << "Cannot spawn enemy: Base not placed." << std::endl;
        return;
    }

    std::cout << "Spawning enemy at: " << position.x << "," << position.y << "," << position.z << std::endl;

    auto enemy = m_Registry->CreateEntity();
    m_Registry->AddComponent(enemy, TransformComponent{
        position,
        glm::vec3{0.12f},
        {0.0f, 0.0f, 0.0f}
        });
    m_Registry->AddComponent(enemy, RenderComponent{ {0.8f, 0.2f, 0.8f, 1.0f} });
    m_Registry->AddComponent(enemy, MeshComponent{ MeshType::Cube });
    m_Registry->AddComponent(enemy, HealthComponent{ 50, 50 });
    m_Registry->AddComponent(enemy, EnemyComponent{});

    m_Registry->AddComponent(enemy, CollisionComponent{ 0.4f });

    MovementComponent move;
    move.speed = 3.0f;
    move.path = {};
    move.currentPathIndex = 0;

    m_Registry->AddComponent(enemy, move);
}


void Game::Init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_PrimaryMonitor = glfwGetPrimaryMonitor();
    m_PrimaryMode = glfwGetVideoMode(m_PrimaryMonitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    m_Window = glfwCreateWindow(m_WindowedWidth, m_WindowedHeight, m_Title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr) {
        glfwTerminate();
        return;
    }



    GLFWimage images[1];
    images[0].pixels = stbi_load("miscellaneous/icon.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(m_Window, 1, images);
    stbi_image_free(images[0].pixels);


    glfwMakeContextCurrent(m_Window);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetCursorPosCallback(m_Window, cursor_pos_callback);
    glfwSetKeyCallback(m_Window, key_callback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return;

    glViewport(0, 0, m_Width, m_Height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_Registry = std::make_unique<ecs::Registry>();

    //register comps
    m_Registry->RegisterComponent<TransformComponent>();
    m_Registry->RegisterComponent<RenderComponent>();
    m_Registry->RegisterComponent<MeshComponent>();
    m_Registry->RegisterComponent<BuildingComponent>();
    m_Registry->RegisterComponent<GhostComponent>();
    m_Registry->RegisterComponent<GridTileComponent>();
    m_Registry->RegisterComponent<SelectableComponent>();
    m_Registry->RegisterComponent<ResourceGeneratorComponent>();
    m_Registry->RegisterComponent<HealthComponent>();
    m_Registry->RegisterComponent<MovementComponent>();
    m_Registry->RegisterComponent<EnemyComponent>();
    m_Registry->RegisterComponent<TurretAIComponent>();
    m_Registry->RegisterComponent<BombComponent>();
    m_Registry->RegisterComponent<ProjectileComponent>();
    m_Registry->RegisterComponent<CollisionComponent>();

    //register systems
    m_RenderSystem = m_Registry->RegisterSystem<RenderSystem>();
    m_UISystem = m_Registry->RegisterSystem<UISystem>();
    m_InputSystem = m_Registry->RegisterSystem<InputSystem>();
    m_ResourceSystem = m_Registry->RegisterSystem<ResourceSystem>();
    m_GridSystem = m_Registry->RegisterSystem<GridSystem>();
    m_MovementSystem = m_Registry->RegisterSystem<MovementSystem>();
    m_EnemyAISystem = m_Registry->RegisterSystem<EnemyAISystem>();
    m_BalanceSystem = m_Registry->RegisterSystem<BalanceSystem>();
    m_CombatSystem = m_Registry->RegisterSystem<CombatSystem>();
    m_ProjectileSystem = m_Registry->RegisterSystem<ProjectileSystem>();
    m_CollisionSystem = m_Registry->RegisterSystem<CollisionSystem>();

    //set signatures
    ecs::Signature renderSig;
    renderSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<MeshComponent>());
    m_Registry->SetSystemSignature<RenderSystem>(renderSig);

    ecs::Signature inputSig;
    inputSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<MeshComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<GhostComponent>());
    m_Registry->SetSystemSignature<InputSystem>(inputSig);

    ecs::Signature uiSig;
    m_Registry->SetSystemSignature<UISystem>(uiSig);

    ecs::Signature resourceSig;
    resourceSig.set(m_Registry->GetComponentTypeID<ResourceGeneratorComponent>());
    m_Registry->SetSystemSignature<ResourceSystem>(resourceSig);

    ecs::Signature moveSig;
    moveSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    moveSig.set(m_Registry->GetComponentTypeID<MovementComponent>());
    m_Registry->SetSystemSignature<MovementSystem>(moveSig);

    ecs::Signature aiSig;
    aiSig.set(m_Registry->GetComponentTypeID<EnemyComponent>());
    aiSig.set(m_Registry->GetComponentTypeID<MovementComponent>());
    m_Registry->SetSystemSignature<EnemyAISystem>(aiSig);

    ecs::Signature combatSig;
    combatSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    combatSig.set(m_Registry->GetComponentTypeID<TurretAIComponent>());
    m_Registry->SetSystemSignature<CombatSystem>(combatSig);

    ecs::Signature balanceSig;
    m_Registry->SetSystemSignature<BalanceSystem>(balanceSig);

    ecs::Signature projectileSig;
    projectileSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    projectileSig.set(m_Registry->GetComponentTypeID<ProjectileComponent>());
    m_Registry->SetSystemSignature<ProjectileSystem>(projectileSig);

    ecs::Signature collisionSig;
    collisionSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    collisionSig.set(m_Registry->GetComponentTypeID<CollisionComponent>());
    m_Registry->SetSystemSignature<CollisionSystem>(collisionSig);


    m_RenderSystem->Init();
    m_UISystem->Init(m_Registry.get());
    m_InputSystem->Init(m_Window, m_Registry.get(), this);
    m_GridSystem->Init();

    m_BalanceSystem->Init();
    m_ResourceSystem->Init(m_BalanceSystem.get(), 1000.0);
    m_EnemyAISystem->Init(m_GridSystem.get());
    m_CombatSystem->Init(m_BalanceSystem.get(), m_ResourceSystem.get(), m_GridSystem.get());
   // m_MovementSystem->Init(m_GridSystem.get());



    int w, h;
    glfwGetFramebufferSize(m_Window, &w, &h);
    glViewport(0, 0, w, h);
    m_Width = w;
    m_Height = h;
    m_InputSystem->SetWindowSize(w, h);


    auto grid = m_Registry->CreateEntity();
    m_Registry->AddComponent(grid, TransformComponent{ {0,0,0}, {20,1,20} });
    m_Registry->AddComponent(grid, RenderComponent{ {0.2f, 0.2f, 0.2f, 1.0f} });
    m_Registry->AddComponent(grid, MeshComponent{ MeshType::Quad });


    auto highlighter = m_Registry->CreateEntity();
    m_Registry->AddComponent(highlighter, TransformComponent{ {0.5f, 0.01f, 0.5f} });
    m_Registry->AddComponent(highlighter, RenderComponent{ {0,0,0,0} });
    m_Registry->AddComponent(highlighter, MeshComponent{ MeshType::None });
    m_Registry->AddComponent(highlighter, GhostComponent{});
}

void Game::ToggleGodMode()
{

    if (m_CurrentState == AppState::PLAYING) {
        m_IsGodMode = !m_IsGodMode;
        m_CheatCodeBuffer.clear();
        m_IsPanning = false;
        m_IsOrbiting = false;

        if (m_IsGodMode) {
            m_PreGodModeTarget = m_OrbitCamera.GetTarget();
            m_PreGodModeDistance = m_OrbitCamera.GetDistance();

            m_FlyCamera.Position = m_OrbitCamera.GetPosition();
            glm::vec3 direction = glm::normalize(m_OrbitCamera.GetTarget() - m_OrbitCamera.GetPosition());
            m_FlyCamera.Yaw = glm::degrees(atan2(direction.z, direction.x));
            m_FlyCamera.Pitch = glm::degrees(asin(direction.y));
            m_FlyCamera.updateCameraVectors();

            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(m_Window, &m_LastMouseX, &m_LastMouseY);
            std::cout << "FREE CAM ACTIVATED" << std::endl;
        }
        else {
            m_OrbitCamera.SetTarget(m_PreGodModeTarget);
            m_OrbitCamera.SetDistance(m_PreGodModeDistance);

            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            std::cout << "FREE CAM DEACTIVATED" << std::endl;
        }
    }

}



void Game::Run() {
    double t = 0.0;
    const double dt = 0.01;
    double currentTime = glfwGetTime();
    double accumulator = 0.0;

    while (!glfwWindowShouldClose(m_Window)) {
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        if (frameTime > 0.25) frameTime = 0.25;
        accumulator += frameTime;

        ProcessInput((float)frameTime);

        int w, h;
        glfwGetFramebufferSize(m_Window, &w, &h);
        if (w == 0 || h == 0) {
            continue;
        }

        switch (m_CurrentState) {

        case AppState::MAIN_MENU:
            m_UISystem->Update((float)dt);
            accumulator = 0.0;
            break;

        case AppState::PLAYING:
            while (accumulator >= dt) {
                m_ResourceSystem->Update((float)dt);
                m_UISystem->Update((float)dt);
                if (!m_IsGodMode) {
                    m_InputSystem->Update();
                }
                if (m_BasePlaced) {
                    auto& enemyEntities = m_EnemyAISystem->m_Entities;
                    auto& renderableEntities = m_RenderSystem->m_Entities;
                    m_EnemyAISystem->Update((float)dt, m_Registry.get(), renderableEntities);
                    m_CombatSystem->Update((float)dt, m_Registry.get(), enemyEntities, renderableEntities);
                    m_ProjectileSystem->Update((float)dt, m_Registry.get(), enemyEntities);
                }
                m_MovementSystem->Update((float)dt);
                m_CollisionSystem->Update((float)dt, m_GridSystem.get());

                accumulator -= dt;
                t += dt;
            }
            break;

        case AppState::PAUSED:
            m_UISystem->Update((float)dt);
            accumulator = 0.0;
            break;
        }

        Render();
    }
}



void Game::ProcessInput(float dt)
{
    glfwPollEvents();


    if (m_IsGodMode)
    {
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::FORWARD, dt);
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::BACKWARD, dt);
        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::LEFT, dt);
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::RIGHT, dt);
        if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::UP, dt);
        if (glfwGetKey(m_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::DOWN, dt);
    }
}

void Game::Update(float dt) { /* (not used) */ }

void Game::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // <-- This is totally fine here

    glm::mat4 projection = m_IsGodMode ?
        glm::perspective(glm::radians(45.0f), (float)m_Width / (float)m_Height, 0.1f, 100.0f) :
        m_OrbitCamera.GetProjectionMatrix((float)m_Width / (float)m_Height);

    glm::mat4 view = m_IsGodMode ?
        m_FlyCamera.GetViewMatrix() :
        m_OrbitCamera.GetViewMatrix();

    glm::vec3 camPos = m_IsGodMode ?
        m_FlyCamera.Position :
        m_OrbitCamera.GetPosition();

    m_InputSystem->UpdateMatrices(projection, view, camPos);

    m_RenderSystem->Render(m_Registry.get(), projection, view);
    m_UISystem->Render(m_Registry.get());

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_Window);
}


void Game::Cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}




void Game::ClearWorld()
{
    m_Registry->Reset();
    m_GridSystem->Init();
    m_BalanceSystem->Init();
    m_ResourceSystem->Init(m_BalanceSystem.get(), 1000);
    m_EnemyAISystem->Init(m_GridSystem.get());
    m_CombatSystem->Init(m_BalanceSystem.get(), m_ResourceSystem.get(), m_GridSystem.get());

    m_BasePlaced = false;
    m_BasePosition = { 0,0,0 };

    auto grid = m_Registry->CreateEntity();
    m_Registry->AddComponent(grid, TransformComponent{ {0,0,0}, {20,1,20} });
    m_Registry->AddComponent(grid, RenderComponent{ {0.2f, 0.2f, 0.2f, 1.0f} });
    m_Registry->AddComponent(grid, MeshComponent{ MeshType::Quad });

    auto highlighter = m_Registry->CreateEntity();
    m_Registry->AddComponent(highlighter, TransformComponent{ {0.5f, 0.01f, 0.5f} });
    m_Registry->AddComponent(highlighter, RenderComponent{ {0,0,0,0} });
    m_Registry->AddComponent(highlighter, MeshComponent{ MeshType::None });
    m_Registry->AddComponent(highlighter, GhostComponent{});
}


void Game::SaveGame()
{
    std::cout << "Saving game..." << std::endl;
    json saveFile;

    saveFile["globals"]["base_placed"] = m_BasePlaced;
    saveFile["globals"]["base_pos"] = m_BasePosition;
    saveFile["globals"]["resources"] = m_ResourceSystem->GetResources();
    saveFile["globals"]["balance"] = m_BalanceSystem->GetBalance();
    saveFile["globals"]["cam_target"] = m_OrbitCamera.GetTarget();
    saveFile["globals"]["cam_dist"] = m_OrbitCamera.GetDistance();

    saveFile["entities"] = json::array();
    for (const auto& entity : m_Registry->GetLivingEntities())
    {
        if (entity == 0 || entity == 1) continue;

        json entityJson;
        entityJson["id"] = entity;

        if (m_Registry->HasComponent<TransformComponent>(entity))
            entityJson["transform"] = m_Registry->GetComponent<TransformComponent>(entity);
        if (m_Registry->HasComponent<RenderComponent>(entity))
            entityJson["render"] = m_Registry->GetComponent<RenderComponent>(entity);
        if (m_Registry->HasComponent<MeshComponent>(entity))
            entityJson["mesh"] = m_Registry->GetComponent<MeshComponent>(entity);
        if (m_Registry->HasComponent<BuildingComponent>(entity))
            entityJson["building"] = m_Registry->GetComponent<BuildingComponent>(entity);
        if (m_Registry->HasComponent<HealthComponent>(entity))
            entityJson["health"] = m_Registry->GetComponent<HealthComponent>(entity);
        if (m_Registry->HasComponent<ResourceGeneratorComponent>(entity))
            entityJson["generator"] = m_Registry->GetComponent<ResourceGeneratorComponent>(entity);
        if (m_Registry->HasComponent<TurretAIComponent>(entity))
            entityJson["turret_ai"] = m_Registry->GetComponent<TurretAIComponent>(entity);
        if (m_Registry->HasComponent<BombComponent>(entity))
            entityJson["bomb"] = m_Registry->GetComponent<BombComponent>(entity);
        if (m_Registry->HasComponent<CollisionComponent>(entity))
            entityJson["collision"] = m_Registry->GetComponent<CollisionComponent>(entity);
        if (m_Registry->HasComponent<EnemyComponent>(entity))
            entityJson["enemy"] = true;
        if (m_Registry->HasComponent<MovementComponent>(entity))
            entityJson["movement"] = m_Registry->GetComponent<MovementComponent>(entity);

        saveFile["entities"].push_back(entityJson);
    }

    std::ofstream o("savegame.json");
    o << saveFile.dump(2);
    o.close();
    std::cout << "Game saved!" << std::endl;
}

void Game::LoadGame()
{
    std::cout << "Loading game..." << std::endl;
    json saveFile;
    std::ifstream i("savegame.json");
    if (!i.is_open()) {
        std::cout << "No save file found." << std::endl;
        return;
    }

    try {
        i >> saveFile;
    }
    catch (json::parse_error& e) {
        std::cout << "Error parsing save file: " << e.what() << std::endl;
        i.close();
        return;
    }
    i.close();

    ClearWorld();

    m_BasePlaced = saveFile["globals"]["base_placed"].get<bool>();
    m_BasePosition = saveFile["globals"]["base_pos"].get<glm::vec3>();
    m_ResourceSystem->AddResources(saveFile["globals"]["resources"].get<double>());
    m_BalanceSystem->m_Balance = saveFile["globals"]["balance"].get<float>();
    m_OrbitCamera.SetTarget(saveFile["globals"]["cam_target"].get<glm::vec3>());
    m_OrbitCamera.SetDistance(saveFile["globals"]["cam_dist"].get<float>());

    for (const auto& entityJson : saveFile["entities"])
    {
        ecs::Entity id = entityJson["id"].get<ecs::Entity>();
        auto entity = m_Registry->CreateEntity(id);

        if (entityJson.contains("transform"))
            m_Registry->AddComponent(entity, entityJson.at("transform").get<TransformComponent>());
        if (entityJson.contains("render"))
            m_Registry->AddComponent(entity, entityJson.at("render").get<RenderComponent>());
        if (entityJson.contains("mesh"))
            m_Registry->AddComponent(entity, entityJson.at("mesh").get<MeshComponent>());
        if (entityJson.contains("building"))
            m_Registry->AddComponent(entity, entityJson.at("building").get<BuildingComponent>());
        if (entityJson.contains("health"))
            m_Registry->AddComponent(entity, entityJson.at("health").get<HealthComponent>());
        if (entityJson.contains("generator"))
            m_Registry->AddComponent(entity, entityJson.at("generator").get<ResourceGeneratorComponent>());
        if (entityJson.contains("turret_ai"))
            m_Registry->AddComponent(entity, entityJson.at("turret_ai").get<TurretAIComponent>());
        if (entityJson.contains("bomb"))
            m_Registry->AddComponent(entity, entityJson.at("bomb").get<BombComponent>());
        if (entityJson.contains("collision"))
            m_Registry->AddComponent(entity, entityJson.at("collision").get<CollisionComponent>());
        if (entityJson.contains("enemy"))
            m_Registry->AddComponent(entity, EnemyComponent{});
        if (entityJson.contains("movement"))
            m_Registry->AddComponent(entity, entityJson.at("movement").get<MovementComponent>());

        if (m_Registry->HasComponent<BuildingComponent>(entity)) {
            auto& t = m_Registry->GetComponent<TransformComponent>(entity);
            glm::ivec2 anchor = m_GridSystem->WorldToGrid(t.position - glm::vec3((t.scale.x / 2.f) - 0.5f, 0.f, (t.scale.z / 2.f) - 0.5f));
            int footprintX = (int)t.scale.x;
            int footprintZ = (int)t.scale.z;
            if (std::abs(t.rotation.y - 90.0f) < 1.0f || std::abs(t.rotation.y - 270.0f) < 1.0f) {
                std::swap(footprintX, footprintZ);
            }
            for (int x = 0; x < footprintX; ++x) {
                for (int z = 0; z < footprintZ; ++z) {
                    m_GridSystem->SetEntityAt(anchor.x + x, anchor.y + z, entity);
                }
            }
        }
    }

    std::cout << "Game loaded!" << std::endl;
    SetAppState(AppState::PLAYING);
}