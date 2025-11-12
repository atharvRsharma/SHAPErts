#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "FlyCamera.h" 


const float DEFAULT_YAW = glm::radians(45.0f);
const float DEFAULT_PITCH = glm::radians(45.0f); 
const float MIN_DISTANCE = 5.0f;
const float MAX_DISTANCE = 40.0f;
const float DEFAULT_DISTANCE = 20.0f;
const float PAN_SPEED = 0.03f;
const float ORBIT_SENSITIVITY = 0.005f;

class OrbitCamera
{
public:
    OrbitCamera(glm::vec3 target = glm::vec3(0.0f)) 
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

    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(m_Position, m_Target, m_WorldUp);
    }
    
    glm::mat4 GetProjectionMatrix(float aspectRatio) const
    {
        return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    }

    glm::vec3 GetPosition() const { return m_Position; }
    glm::vec3 GetTarget() const { return m_Target; }

    void ProcessMousePan(float xoffset, float yoffset)
    {
        glm::vec3 panRight = glm::normalize(glm::vec3(cos(m_Yaw), 0.0f, -sin(m_Yaw)));
        glm::vec3 panForward = glm::normalize(glm::vec3(sin(m_Yaw), 0.0f, cos(m_Yaw)));

        float distanceFactor = m_Distance / MAX_DISTANCE;

        m_Target -= panRight * xoffset * m_PanSpeed * distanceFactor;

        m_Target -= panForward * yoffset * m_PanSpeed * distanceFactor;

        updateCameraPosition();
    }

    void ProcessMouseOrbit(float xoffset)
    {
        m_Yaw -= xoffset * m_OrbitSensitivity;
        updateCameraPosition();
    }

    void ProcessMouseScroll(float yoffset)
    {
        m_Distance -= yoffset;
        m_Distance = std::max(MIN_DISTANCE, std::min(m_Distance, MAX_DISTANCE));
        updateCameraPosition();
    }

    void SyncFrom(const FlyCamera& flyCam)
    {
        m_Position = flyCam.Position;

        m_Target = flyCam.Position + flyCam.Front * 10.0f;

        glm::vec3 direction = glm::normalize(m_Position - m_Target);
        m_Distance = glm::length(m_Position - m_Target);
        m_Pitch = asin(direction.y);
        m_Yaw = atan2(direction.x, direction.z);
    }

private:
    void updateCameraPosition()
    {
        m_Position.x = m_Target.x + (m_Distance * cos(m_Pitch)) * sin(m_Yaw);
        m_Position.z = m_Target.z + (m_Distance * cos(m_Pitch)) * cos(m_Yaw);
        m_Position.y = m_Target.y + m_Distance * sin(m_Pitch);
    }

    // Members
    glm::vec3 m_Position;
    glm::vec3 m_Target;
    glm::vec3 m_WorldUp;
    float m_Distance, m_Yaw, m_Pitch;
    float m_PanSpeed, m_OrbitSensitivity;
};