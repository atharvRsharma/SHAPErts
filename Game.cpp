// All elements are generic placeholders for testing purposes.
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
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->m_Camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void Game::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS) {
            game->m_IsOrbiting = true;
            glfwGetCursorPos(window, &game->m_LastMouseX, &game->m_LastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            game->m_IsOrbiting = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
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
    float yoffset = static_cast<float>(ypos - game->m_LastMouseY);

    game->m_LastMouseX = xpos;
    game->m_LastMouseY = ypos;

    if (game->m_IsOrbiting)
    {
        game->m_Camera.ProcessMouseOrbit(xoffset);
    }
    else if (game->m_IsPanning)
    {
        game->m_Camera.ProcessMousePan(xoffset, yoffset);
    }
}


Game::Game(int width, int height, const std::string& title)
    : m_Window(nullptr), m_Width(width), m_Height(height), m_Title(title),
    m_Camera(glm::vec3(0.0f))
{
    Init();
}

Game::~Game() {
    Cleanup();
}

void Game::Init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_Window);

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetCursorPosCallback(m_Window, cursor_pos_callback);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glViewport(0, 0, m_Width, m_Height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_Registry = std::make_unique<ecs::Registry>();

    //register components
    m_Registry->RegisterComponent<TransformComponent>();
    m_Registry->RegisterComponent<RenderComponent>();
    m_Registry->RegisterComponent<MeshComponent>();
    m_Registry->RegisterComponent<GridTileComponent>();
    m_Registry->RegisterComponent<HealthComponent>();
    m_Registry->RegisterComponent<ResourceGeneratorComponent>();
    m_Registry->RegisterComponent<BuildingComponent>();
    m_Registry->RegisterComponent<UnitComponent>();
    m_Registry->RegisterComponent<MovementComponent>();
    m_Registry->RegisterComponent<SelectableComponent>();

    //register systems
    m_RenderSystem = m_Registry->RegisterSystem<RenderSystem>();
    m_UISystem = m_Registry->RegisterSystem<UISystem>();
    m_InputSystem = m_Registry->RegisterSystem<InputSystem>();

    // --- 1. FIXED SYSTEM SIGNATURES ---
    ecs::Signature renderSig;
    renderSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<MeshComponent>()); // <-- ADDED
    m_Registry->SetSystemSignature<RenderSystem>(renderSig);

    ecs::Signature uiSig;
    // (This is fine for now, but it's not really doing anything)
    uiSig.set(m_Registry->GetComponentTypeID<SelectableComponent>());
    m_Registry->SetSystemSignature<UISystem>(uiSig);

    // InputSystem just needs the Highlighter's components
    ecs::Signature inputSig;
    inputSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<SelectableComponent>());
    m_Registry->SetSystemSignature<InputSystem>(inputSig);


    // --- 2. FIXED SYSTEM INIT ---
    m_RenderSystem->Init();
    m_UISystem->Init(m_Registry.get());

    m_InputSystem->Init(m_Window, m_Registry.get()); // <-- 2-arg version
    m_InputSystem->SetWindowSize(m_Width, m_Height); // <-- Set initial size


    // --- 3. FIXED ENTITY CREATION ---

    // 1. Create the Grid (1 entity)
    auto grid = m_Registry->CreateEntity();
    m_Registry->AddComponent(grid, TransformComponent{
        {0.0f, 0.0f, 0.0f},     // Position
        {20.0f, 1.0f, 20.0f},   // Scale (20x1x20)
        {0.0f, 0.0f, 0.0f}      // Rotation
        });
    m_Registry->AddComponent(grid, RenderComponent{
        {0.2f, 0.2f, 0.2f, 1.0f} // Dark grey
        });
    m_Registry->AddComponent(grid, MeshComponent{
        MeshType::Quad
        });

    // 2. Create the Highlighter
    auto highlighter = m_Registry->CreateEntity();
    m_Registry->AddComponent(highlighter, TransformComponent{
        {0.5f, 0.01f, 0.5f}, // Default position
        {1.0f, 1.0f, 1.0f},  // 1x1x1 scale
        {0.0f, 0.0f, 0.0f}
        });
    m_Registry->AddComponent(highlighter, RenderComponent{
        {0.0f, 0.0f, 0.0f, 0.0f} // Invisible by default
        });
    m_Registry->AddComponent(highlighter, MeshComponent{
        MeshType::Quad
        });
    m_Registry->AddComponent(highlighter, SelectableComponent{});

    // 3. Create our test "Soldier"
    auto soldier = m_Registry->CreateEntity();
    m_Registry->AddComponent(soldier, TransformComponent{
        { -2.0f, 0.5f, -2.0f }, // Pos (y=0.5 so base is on ground)
        { 1.0f, 1.0f, 1.0f },  // Scale (will be ignored by soldier render)
        { 0.0f, 0.0f, 0.0f }
        });
    m_Registry->AddComponent(soldier, RenderComponent{
        {0.8f, 0.2f, 0.2f, 1.0f} // Red (for body)
        });
    m_Registry->AddComponent(soldier, MeshComponent{
        MeshType::Unit_Soldier
        });
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

        if (frameTime > 0.25) {
            frameTime = 0.25;
        }

        accumulator += frameTime;

        ProcessInput();

        while (accumulator >= dt) {
            m_UISystem->Update((float)dt);
            m_InputSystem->Update();
            accumulator -= dt;
            t += dt;
        }

        Render();
    }
}

void Game::ProcessInput()
{
    glfwPollEvents();

    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_Window, true);
    }
}

void Game::Update(float dt) {
    // (not used)
}

void Game::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = m_Camera.GetProjectionMatrix((float)m_Width / (float)m_Height);
    glm::mat4 view = m_Camera.GetViewMatrix();

    m_InputSystem->UpdateMatrices(projection, view, m_Camera.GetPosition());

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