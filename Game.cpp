#include "Game.h"
#include "ECS.h"
#include "Components.h"
#include "Systems.h"
#include "RenderSystem.h"
#include "UISystem.h"
#include "InputSystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ResourceSystem.h"
#include "GridSystem.h"

// --- Callbacks ---

void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
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
    if (ImGui::GetIO().WantCaptureMouse) return;
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game && !game->m_IsGodMode) {
        game->m_OrbitCamera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void Game::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    // --- NEW "Click-Away to Cancel" LOGIC ---
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
        ImGui::GetIO().WantCaptureMouse && game->m_InputSystem->IsInBuildMode())
    {}

    if (ImGui::GetIO().WantCaptureMouse) return; // All other UI clicks stop here

    if (game->m_IsGodMode) return;

    // --- (Pan/Orbit logic is unchanged) ---
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
}

void Game::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    float xoffset = static_cast<float>(xpos - game->m_LastMouseX);
    float yoffset = static_cast<float>(game->m_LastMouseY - ypos); 

    if (game->m_IsGodMode || game->m_IsOrbiting || game->m_IsPanning) {
        game->m_LastMouseX = xpos;
        game->m_LastMouseY = ypos;
    }

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

    game->m_KeyCodeBuffer.push_back(key);

    if (game->m_KeyCodeBuffer.size() > game->m_GodModeCode.size()) {
        game->m_KeyCodeBuffer.erase(game->m_KeyCodeBuffer.begin());
    }

    if (game->m_KeyCodeBuffer == game->m_GodModeCode) {
        game->ToggleGodMode();
    }
}

Game::Game(int width, int height, const std::string& title)
    : m_Window(nullptr), m_Width(width), m_Height(height), m_Title(title),
    m_OrbitCamera(glm::vec3(0.0f)),
    m_FlyCamera(glm::vec3(0.0f, 15.0f, 15.0f)),
    m_CurrentState(AppState::PLAYING)
{
    Init();
}

Game::~Game() { Cleanup(); }

void Game::SetAppState(AppState newState) {
    m_CurrentState = newState;
    if (m_CurrentState == AppState::PLAYING) {
        std::cout << "Base placed! Game is now PLAYING." << std::endl;
    }
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

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr) {
        glfwTerminate();
        return;
    }
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

    m_Registry->RegisterComponent<TransformComponent>();
    m_Registry->RegisterComponent<RenderComponent>();
    m_Registry->RegisterComponent<MeshComponent>();
    m_Registry->RegisterComponent<BuildingComponent>();
    m_Registry->RegisterComponent<GridTileComponent>();
    m_Registry->RegisterComponent<SelectableComponent>();

    m_RenderSystem = m_Registry->RegisterSystem<RenderSystem>();
    m_UISystem = m_Registry->RegisterSystem<UISystem>();
    m_InputSystem = m_Registry->RegisterSystem<InputSystem>();
    m_ResourceSystem = m_Registry->RegisterSystem<ResourceSystem>();
    m_GridSystem = m_Registry->RegisterSystem<GridSystem>();

    ecs::Signature renderSig;
    renderSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<MeshComponent>());
    m_Registry->SetSystemSignature<RenderSystem>(renderSig);

    ecs::Signature inputSig;
    inputSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<SelectableComponent>());
    m_Registry->SetSystemSignature<InputSystem>(inputSig);

    ecs::Signature uiSig;
    m_Registry->SetSystemSignature<UISystem>(uiSig);

    m_RenderSystem->Init();
    m_UISystem->Init(m_Registry.get());
    m_InputSystem->Init(m_Window, m_Registry.get(), this); 
    m_InputSystem->SetWindowSize(m_Width, m_Height);
    m_ResourceSystem->Init(1000.0);
    m_GridSystem->Init();

    auto grid = m_Registry->CreateEntity();
    m_Registry->AddComponent(grid, TransformComponent{ {0,0,0}, {20,1,20} });
    m_Registry->AddComponent(grid, RenderComponent{ {0.2f, 0.2f, 0.2f, 1.0f} });
    m_Registry->AddComponent(grid, MeshComponent{ MeshType::Quad });

    auto highlighter = m_Registry->CreateEntity();
    m_Registry->AddComponent(highlighter, TransformComponent{ {0.5f, 0.01f, 0.5f} });
    m_Registry->AddComponent(highlighter, RenderComponent{ {0,0,0,0} });
    m_Registry->AddComponent(highlighter, MeshComponent{ MeshType::Quad });
    m_Registry->AddComponent(highlighter, SelectableComponent{});

    //m_InputSystem->EnterBuildMode(MeshType::Pyramid);

}

void Game::ToggleGodMode()
{
    m_IsGodMode = !m_IsGodMode;
    m_KeyCodeBuffer.clear();
    m_IsPanning = false;
    m_IsOrbiting = false;

    if (m_IsGodMode) {
        // --- SAVE THE ORBIT CAM STATE ---
        m_PreGodModeTarget = m_OrbitCamera.GetTarget();
        m_PreGodModeDistance = m_OrbitCamera.GetDistance();

        // Sync fly cam to orbit cam
        m_FlyCamera.Position = m_OrbitCamera.GetPosition();

        glm::vec3 direction = glm::normalize(m_OrbitCamera.GetTarget() - m_OrbitCamera.GetPosition());
        m_FlyCamera.Yaw = glm::degrees(atan2(direction.z, direction.x));
        m_FlyCamera.Pitch = glm::degrees(asin(direction.y));
        m_FlyCamera.updateCameraVectors();

        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(m_Window, &m_LastMouseX, &m_LastMouseY);
        std::cout << "GOD MODE: ACTIVATED" << std::endl;
    }
    else {
        // --- RESTORE THE ORBIT CAM STATE ---
        m_OrbitCamera.SetTarget(m_PreGodModeTarget);
        m_OrbitCamera.SetDistance(m_PreGodModeDistance);

        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        std::cout << "GOD MODE: DEACTIVATED" << std::endl;
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

        switch (m_CurrentState) {
        //case AppState::BASE_PLACEMENT: // <-- RENAMED
        case AppState::PLAYING:        // <-- RENAMED
        {
            while (accumulator >= dt) {
                m_ResourceSystem->Update((float)dt);
                m_UISystem->Update((float)dt);

                if (!m_IsGodMode) {
                    m_InputSystem->Update();
                }

                accumulator -= dt;
                t += dt;
            }
            Render();
            break;
        }
        }
    }
}

void Game::ProcessInput(float dt)
{
    glfwPollEvents();
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_Window, true);
    }

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
        if (glfwGetKey(m_Window, GLFW_KEY_E) == GLFW_PRESS)
            m_FlyCamera.ProcessKeyboard(FlyCam_Movement::UP, dt);
        if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
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