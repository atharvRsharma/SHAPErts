#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <vector>

#include "OrbitCamera.h" // <-- Renamed
#include "FlyCamera.h"   // <-- New

// Forward declarations
namespace ecs { class Registry; }
class RenderSystem;
class UISystem;
class InputSystem;

class Game {
public:
    Game(int width, int height, const std::string& title);
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void Run();

    // --- Camera Members ---
    OrbitCamera m_OrbitCamera; // <-- Renamed
    FlyCamera m_FlyCamera;     // <-- New
    bool m_IsGodMode = false;

    // --- Public window for UISystem ---
    GLFWwindow* m_Window;

private:
    void Init();
    void ProcessInput(float dt); // <-- Re-added dt
    void Update(float dt);
    void Render();
    void Cleanup();
    void ToggleGodMode(); // <-- New

    int m_Width, m_Height;
    std::string m_Title;

    // --- ECS ---
    std::unique_ptr<ecs::Registry> m_Registry;

    // --- Systems ---
    std::shared_ptr<RenderSystem> m_RenderSystem;
    std::shared_ptr<UISystem> m_UISystem;
    std::shared_ptr<InputSystem> m_InputSystem;

    // --- Mouse State ---
    bool m_IsPanning = false;
    bool m_IsOrbiting = false;
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    // --- Cheat Code ---
    const std::vector<int> m_GodModeCode = { GLFW_KEY_F, GLFW_KEY_C, GLFW_KEY_G, GLFW_KEY_V };
    std::vector<int> m_KeyCodeBuffer;

    // --- Callbacks ---
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // <-- New
};