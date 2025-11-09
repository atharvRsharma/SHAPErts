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

// ImGui Includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// --- GLFW Callback Implementations ---

void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->m_Width = width;
        game->m_Height = height;
    }
}

void Game::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // --- Prevent UI from capturing scroll ---
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

    // --- UPDATED: Right mouse for ORBIT ---
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

    // Middle mouse for panning
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

    // Calculate offset since last frame
    float xoffset = static_cast<float>(xpos - game->m_LastMouseX);
    float yoffset = static_cast<float>(ypos - game->m_LastMouseY);

    // Update last mouse position
    game->m_LastMouseX = xpos;
    game->m_LastMouseY = ypos;

    // --- UPDATED: Handle Orbit and Pan ---
    if (game->m_IsOrbiting)
    {
        // We only pass x-offset for side-to-side orbit
        game->m_Camera.ProcessMouseOrbit(xoffset);
    }
    else if (game->m_IsPanning)
    {
        game->m_Camera.ProcessMousePan(xoffset, yoffset);
    }
}


Game::Game(int width, int height, const std::string& title)
// --- UPDATED: New Camera Constructor ---
    : m_Window(nullptr), m_Width(width), m_Height(height), m_Title(title),
    m_Camera(glm::vec3(0.0f)) // Init camera targeting origin
{
    Init();
}

Game::~Game() {
    Cleanup();
}

void Game::Init() {
    // --- GLFW/GLAD Setup ---
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

    // --- SET WINDOW USER POINTER AND CALLBACKS ---
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetScrollCallback(m_Window, scroll_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSetCursorPosCallback(m_Window, cursor_pos_callback);

    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glViewport(0, 0, m_Width, m_Height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST); // --- ENABLE DEPTH TESTING ---

    // --- ImGui Setup ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- ECS Setup ---
    m_Registry = std::make_unique<ecs::Registry>();

    // Register components
    m_Registry->RegisterComponent<TransformComponent>();
    m_Registry->RegisterComponent<RenderComponent>();
    m_Registry->RegisterComponent<GridTileComponent>();
    m_Registry->RegisterComponent<HealthComponent>();
    m_Registry->RegisterComponent<ResourceGeneratorComponent>();
    m_Registry->RegisterComponent<BuildingComponent>();
    m_Registry->RegisterComponent<UnitComponent>();
    m_Registry->RegisterComponent<MovementComponent>();
    m_Registry->RegisterComponent<SelectableComponent>();

    // Register systems
    m_RenderSystem = m_Registry->RegisterSystem<RenderSystem>();
    m_UISystem = m_Registry->RegisterSystem<UISystem>();
    m_InputSystem = m_Registry->RegisterSystem<InputSystem>();

    // --- Set System Signatures ---
    ecs::Signature renderSig;
    renderSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    renderSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    m_Registry->SetSystemSignature<RenderSystem>(renderSig);

    ecs::Signature uiSig;
    uiSig.set(m_Registry->GetComponentTypeID<SelectableComponent>());
    uiSig.set(m_Registry->GetComponentTypeID<GridTileComponent>());
    m_Registry->SetSystemSignature<UISystem>(uiSig);

    ecs::Signature inputSig;
    inputSig.set(m_Registry->GetComponentTypeID<TransformComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<GridTileComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<SelectableComponent>());
    inputSig.set(m_Registry->GetComponentTypeID<RenderComponent>());
    m_Registry->SetSystemSignature<InputSystem>(inputSig);


    // --- Init Systems ---
    m_RenderSystem->Init(); // No longer takes projection
    m_UISystem->Init(m_Registry.get());

    // Input system needs the projection, but it will be out of date
    // as the camera moves. We will fix this next.
    glm::mat4 projection = m_Camera.GetProjectionMatrix((float)m_Width / (float)m_Height);
    m_InputSystem->Init(m_Window, m_Registry.get(), m_Width, m_Height);

    // --- Create Initial Entities (The Grid) ---
    const int gridSize = 20;
    for (int x = 0; x < gridSize; ++x) {
        for (int y = 0; y < gridSize; ++y) {
            auto tile = m_Registry->CreateEntity();

            // Position centered around (0,0) on the XZ plane
            float posX = (float)x - (float)gridSize / 2.0f + 0.5f;
            float posZ = (float)y - (float)gridSize / 2.0f + 0.5f; // Now Z, not Y

            m_Registry->AddComponent(tile, TransformComponent{
                {posX, 0.0f, posZ}, // Position on XZ plane
                {0.95f, 0.95f}
                });

            m_Registry->AddComponent(tile, RenderComponent{
                {0.2f, 0.2f, 0.2f, 1.0f} // Dark grey color
                });

            m_Registry->AddComponent(tile, GridTileComponent{
                x, y, false
                });

            m_Registry->AddComponent(tile, SelectableComponent{
                false
                });
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

        if (frameTime > 0.25) {
            frameTime = 0.25;
        }

        accumulator += frameTime;

        // --- Process Input (once per frame) ---
        ProcessInput(); // Polls for ESC key

        // --- Fixed Update (Simulation) ---
        while (accumulator >= dt) {
            // We pass dt (fixed step) to simulation systems
            m_UISystem->Update((float)dt);

            // Handle game input (selection)
            // We run this in the simulation step
            m_InputSystem->Update();

            accumulator -= dt;
            t += dt;
        }

        // --- Render (once per frame) ---
        Render();
    }
}

void Game::ProcessInput()
{
    glfwPollEvents();

    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_Window, true);
    }

    // All camera input is now handled by callbacks
}

void Game::Update(float dt) {
    // This function is for the simulation step
    // We call its contents directly from Run() for now
}

void Game::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = m_Camera.GetProjectionMatrix((float)m_Width / (float)m_Height);
    glm::mat4 view = m_Camera.GetViewMatrix();

    // --- THIS IS THE FIX ---
    // Pass all camera info to the InputSystem
    m_InputSystem->UpdateMatrices(projection, view, m_Camera.GetPosition());

    // Run render systems
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