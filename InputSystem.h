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
    void Init(GLFWwindow* window, ecs::Registry* registry, int width, int height) {
        m_Window = window;
        // m_Registry is set by base class
        m_Width = width;
        m_Height = height;
    }

    // --- CHANGED: Now needs camera position ---
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

private:
    GLFWwindow* m_Window;
    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_InvProjection;
    glm::mat4 m_InvView;
    glm::vec3 m_CameraPosition; // --- NEW ---
    int m_Width;
    int m_Height;
    bool m_MousePressedLastFrame = false;

    // --- NEW: Raycasting function ---
    glm::vec3 ScreenToWorldRay(double xpos, double ypos) {
        // 1. Screen space to NDC
        float ndcX = (2.0f * (float)xpos) / (float)m_Width - 1.0f;
        float ndcY = 1.0f - (2.0f * (float)ypos) / (float)m_Height;

        // 2. NDC to Clip space
        glm::vec4 clipCoords(ndcX, ndcY, -1.0f, 1.0f); // -1.0f for "near"

        // 3. Clip space to View space
        glm::vec4 eyeCoords = m_InvProjection * clipCoords;
        eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

        // 4. View space to World space
        glm::vec4 worldRay = m_InvView * eyeCoords;

        return glm::normalize(glm::vec3(worldRay));
    }

    // --- NEW: Ray-Plane Intersection function ---
    std::optional<glm::vec3> IntersectRayWithPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& planeNormal, const glm::vec3& planePoint) {
        float denom = glm::dot(planeNormal, rayDirection);

        // Check if ray is parallel to the plane
        if (std::abs(denom) > 1e-6) {
            float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
            if (t >= 0) {
                // Return intersection point
                return rayOrigin + rayDirection * t;
            }
        }
        // No intersection
        return std::nullopt;
    }


    // --- COMPLETELY REWRITTEN ---
    void HandleMouseClick() {
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);

        // 1. Get the 3D ray from the mouse
        glm::vec3 rayDirection = ScreenToWorldRay(xpos, ypos);
        glm::vec3 rayOrigin = m_CameraPosition;

        // 2. Find where this ray hits our grid plane (y=0)
        glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
        glm::vec3 planePoint(0.0f, 0.0f, 0.0f);

        std::optional<glm::vec3> intersection = IntersectRayWithPlane(rayOrigin, rayDirection, planeNormal, planePoint);

        if (!intersection) {
            // Mouse is not pointing at the grid plane
            return;
        }

        glm::vec3 worldCoords = *intersection;

        // 3. Find which tile was clicked using the 3D world coordinates
        ecs::Entity selectedEntity = ecs::MAX_ENTITIES;

        for (auto const& entity : m_Entities) {
            auto& transform = m_Registry->GetComponent<TransformComponent>(entity);

            float halfWidth = transform.scale.x / 2.0f;
            float halfDepth = transform.scale.y / 2.0f; // Scale.y is our "depth"

            // AABB check on the XZ plane
            if (worldCoords.x >= transform.position.x - halfWidth &&
                worldCoords.x <= transform.position.x + halfWidth &&
                worldCoords.z >= transform.position.z - halfDepth && // <-- Check Z, not Y
                worldCoords.z <= transform.position.z + halfDepth)
            {
                selectedEntity = entity;
                break;
            }
        }

        // 4. Update component state based on selection
        for (auto const& entity : m_Entities) {
            auto& selectable = m_Registry->GetComponent<SelectableComponent>(entity);
            auto& render = m_Registry->GetComponent<RenderComponent>(entity);

            if (entity == selectedEntity) {
                selectable.isSelected = true;
                render.color = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
            }
            else {
                selectable.isSelected = false;
                render.color = { 0.2f, 0.2f, 0.2f, 1.0f }; // Dark grey
            }
        }
    };
};