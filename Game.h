// All elements are generic placeholders for testing purposes.
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

#include "Camera.h" 

// Forward declarations
namespace ecs {
    class Registry;
}
class RenderSystem;
class UISystem;
class InputSystem;

class Game {
public:
    Game(int width, int height, const std::string& title);
    ~Game();

    // Non-copyable
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void Run();

    Camera m_Camera;

private:
    void Init();
    void ProcessInput();
    void Update(float dt);
    void Render();
    void Cleanup();

    GLFWwindow* m_Window;
    int m_Width;
    int m_Height;
    std::string m_Title;

    // ECS
    std::unique_ptr<ecs::Registry> m_Registry;

    // Systems
    std::shared_ptr<RenderSystem> m_RenderSystem;
    std::shared_ptr<UISystem> m_UISystem;
    std::shared_ptr<InputSystem> m_InputSystem;

    // --- MOUSE STATE ---
    bool m_IsPanning = false;
    bool m_IsOrbiting = false; // <-- RENAMED (was m_IsRotating)
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    // --- CALLBACKS ---
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
};