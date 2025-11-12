#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <vector>

#include "OrbitCamera.h"
#include "FlyCamera.h"

// Forward declarations
namespace ecs { class Registry; }
class RenderSystem;
class UISystem;
class InputSystem;
class ResourceSystem;
class GridSystem;

enum class AppState {
    PLAYING
};

class Game {
public:
    Game(int width, int height, const std::string& title);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void Run();

    OrbitCamera m_OrbitCamera;
    FlyCamera m_FlyCamera;
    bool m_IsGodMode = false;
    GLFWwindow* m_Window;

    bool m_BasePlaced = false;
    void OnBasePlaced() { m_BasePlaced = true; }

    void SetAppState(AppState newState);

private:
    void Init();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    void Cleanup();
    void ToggleGodMode();

    int m_Width, m_Height;
    std::string m_Title;
    AppState m_CurrentState;

    std::unique_ptr<ecs::Registry> m_Registry;

    std::shared_ptr<RenderSystem> m_RenderSystem;
    std::shared_ptr<UISystem> m_UISystem;
    std::shared_ptr<InputSystem> m_InputSystem;
    std::shared_ptr<ResourceSystem> m_ResourceSystem;
    std::shared_ptr<GridSystem> m_GridSystem;

    bool m_IsPanning = false;
    bool m_IsOrbiting = false;
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    // --- NEW: Camera state for restore ---
    glm::vec3 m_PreGodModeTarget;
    float m_PreGodModeDistance;

    const std::vector<int> m_GodModeCode = { GLFW_KEY_F, GLFW_KEY_C, GLFW_KEY_V };
    std::vector<int> m_KeyCodeBuffer;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};