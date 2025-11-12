// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include "ECS.h"
#include "Components.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <optional> 
#include "imgui.h" 

class InputSystem : public ecs::System {
public:
    // --- THIS IS THE FIX ---
    GLFWwindow* m_Window; // No longer private

    void Init(GLFWwindow* window, ecs::Registry* registry) {
        m_Window = window;
    }

    void SetWindowSize(int width, int height) {
        m_Width = width;
        m_Height = height;
    }

    void UpdateMatrices(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPosition) {
        m_Projection = projection;
        m_View = view;
        m_InvProjection = glm::inverse(projection);
        m_InvView = glm::inverse(view);
        m_CameraPosition = cameraPosition;
    }

    void Update() {
        if (ImGui::GetIO().WantCaptureMouse) {
            m_MousePressedLastFrame = false; 
            return;
        }

        bool mousePressed = glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        if (mousePressed && !m_MousePressedLastFrame) {
            HandleMouseClick();
        }

        m_MousePressedLastFrame = mousePressed;
    }

    glm::ivec2 GetSelectedGridCoords() const {
        return { m_SelectedGridX, m_SelectedGridY };
    }

private:
    // m_Window was here
    glm::mat4 m_Projection;
    glm::mat4 m_View; 
    glm::mat4 m_InvProjection;
    glm::mat4 m_InvView; 
    glm::vec3 m_CameraPosition; 
    
    int m_Width = 0;
    int m_Height = 0;
    
    bool m_MousePressedLastFrame = false;

    int m_SelectedGridX = -1;
    int m_SelectedGridY = -1;

    glm::vec3 ScreenToWorldRay(double xpos, double ypos) {
        float ndcX = (2.0f * (float)xpos) / (float)m_Width - 1.0f;
        float ndcY = 1.0f - (2.0f * (float)ypos) / (float)m_Height;
        
        glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 eyeCoords = m_InvProjection * clipCoords;
        eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
        glm::vec4 worldRay = m_InvView * eyeCoords;
        
        return glm::normalize(glm::vec3(worldRay));
    }

    std::optional<glm::vec3> IntersectRayWithPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& planeNormal, const glm::vec3& planePoint) {
        float denom = glm::dot(planeNormal, rayDirection);

        if (std::abs(denom) > 1e-6) {
            float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
            if (t >= 0) {
                return rayOrigin + rayDirection * t;
            }
        }
        return std::nullopt;
    }

    void HandleMouseClick() {
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);

        glm::vec3 rayDirection = ScreenToWorldRay(xpos, ypos);
        glm::vec3 rayOrigin = m_CameraPosition;

        glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
        glm::vec3 planePoint(0.0f, 0.0f, 0.0f);

        std::optional<glm::vec3> intersection = IntersectRayWithPlane(rayOrigin, rayDirection, planeNormal, planePoint);

        m_SelectedGridX = -1;
        m_SelectedGridY = -1;

        ecs::Entity selectedEntity = ecs::MAX_ENTITIES; 
        int selectedX = 0;
        int selectedY = 0;

        if (intersection) {
            glm::vec3 worldCoords = *intersection;
            
            selectedX = static_cast<int>(std::floor(worldCoords.x + 10.0f));
            selectedY = static_cast<int>(std::floor(worldCoords.z + 10.0f));

            for (auto const& entity : m_Entities) {
                selectedEntity = entity;
                break;
            }
        }
        
        if (selectedEntity != ecs::MAX_ENTITIES) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(selectedEntity);
            auto& render = m_Registry->GetComponent<RenderComponent>(selectedEntity);

            if (selectedX >= 0 && selectedX < 20 && selectedY >= 0 && selectedY < 20) {
                m_SelectedGridX = selectedX;
                m_SelectedGridY = selectedY;

                transform.position.x = (float)selectedX - 10.0f + 0.5f;
                transform.position.z = (float)selectedY - 10.0f + 0.5f;
                transform.position.y = 0.01f; 
                render.color = {1.0f, 1.0f, 0.0f, 0.5f};
            } else {
                render.color = {0.0f, 0.0f, 0.0f, 0.0f};
            }
        }
    }
};