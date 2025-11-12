// All elements are generic placeholders for testing purposes.
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

// --- NEW CAMERA CONSTANTS ---
constexpr float DEFAULT_YAW = glm::radians(45.0f);

// --- THIS IS THE FIX ---
constexpr float DEFAULT_PITCH = glm::radians(45.0f); // Was -45.0f
// --- END OF FIX ---

constexpr float MIN_DISTANCE = 5.0f;
constexpr float MAX_DISTANCE = 40.0f;
constexpr float DEFAULT_DISTANCE = 20.0f;
constexpr float PAN_SPEED = 0.03f;
constexpr float ORBIT_SENSITIVITY = 0.005f;


class Camera
{
public:
    Camera(glm::vec3 target = glm::vec3(0.0f))
        : m_Target(target),
        m_Distance(DEFAULT_DISTANCE),
        m_Yaw(DEFAULT_YAW),
        m_Pitch(DEFAULT_PITCH),
        m_WorldUp(0.0f, 1.0f, 0.0f),
        m_PanSpeed(PAN_SPEED),
        m_OrbitSensitivity(ORBIT_SENSITIVITY)
    {
        updateCameraPosition();
    }

    // Returns the view matrix (looks *at* the target from our position)
    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(m_Position, m_Target, m_WorldUp);
    }

    // Returns projection (FOV is now fixed, zoom is distance)
    glm::mat4 GetProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    }

    glm::vec3 GetPosition() const { return m_Position; }

    // --- NEW PANNING (MOVES TARGET) ---
    void ProcessMousePan(float xoffset, float yoffset)
    {
        // We pan on the XZ plane, relative to the camera's rotation
        // 1. Get the camera's "right" and "forward" vectors on the flat plane
        glm::vec3 panRight = glm::normalize(glm::vec3(cos(m_Yaw), 0.0f, -sin(m_Yaw)));
        glm::vec3 panForward = glm::normalize(glm::vec3(sin(m_Yaw), 0.0f, cos(m_Yaw)));

        // 2. Adjust pan speed based on how "zoomed out" we are
        float distanceFactor = m_Distance / MAX_DISTANCE;

        // 3. Move the target
        // Pan Left/Right
        m_Target -= panRight * xoffset * m_PanSpeed * distanceFactor;

        // Pan Forward/Back (this was the inverted one, now fixed)
        // Drag "down" (positive yoffset) moves target "down" (towards camera)
        m_Target -= panForward * yoffset * m_PanSpeed * distanceFactor;

        updateCameraPosition();
    }

    // --- NEW ORBIT (MOVES YAW) ---
    void ProcessMouseOrbit(float xoffset)
    {
        // Only orbit side-to-side
        m_Yaw -= xoffset * m_OrbitSensitivity;
        updateCameraPosition();
    }

    // --- NEW ZOOM (MOVES DISTANCE) ---
    void ProcessMouseScroll(float yoffset)
    {
        // Scroll "up" (positive yoffset) zooms in (decreases distance)
        m_Distance -= yoffset;
        m_Distance = std::max(MIN_DISTANCE, std::min(m_Distance, MAX_DISTANCE));
        updateCameraPosition();
    }

private:
    void updateCameraPosition()
    {
        // Calculate the camera's new position in 3D space
        // based on the spherical coordinates around the target
        m_Position.x = m_Target.x + (m_Distance * cos(m_Pitch)) * sin(m_Yaw);
        m_Position.z = m_Target.z + (m_Distance * cos(m_Pitch)) * cos(m_Yaw);
        m_Position.y = m_Target.y + m_Distance * sin(m_Pitch); // sin(45.0) is positive
    }

    // Members
    glm::vec3 m_Position; // Calculated
    glm::vec3 m_Target;   // The focal point
    glm::vec3 m_WorldUp;

    float m_Distance;
    float m_Yaw;    // Orbit angle
    float m_Pitch;  // Slant angle

    float m_PanSpeed;
    float m_OrbitSensitivity;
};