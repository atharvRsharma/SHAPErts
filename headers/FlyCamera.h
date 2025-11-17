#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class FlyCam_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const float FLY_YAW = -90.0f;
const float FLY_PITCH = 0.0f;
const float FLY_SPEED = 10.0f;
const float FLY_SENSITIVITY = 0.1f;

class FlyCamera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    //euler angles
    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;

    FlyCamera(glm::vec3 position = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = FLY_YAW, float pitch = FLY_PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(FLY_SPEED), MouseSensitivity(FLY_SENSITIVITY)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(FlyCam_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FlyCam_Movement::FORWARD)
            Position += Front * velocity;
        if (direction == FlyCam_Movement::BACKWARD)
            Position -= Front * velocity;
        if (direction == FlyCam_Movement::LEFT)
            Position -= Right * velocity;
        if (direction == FlyCam_Movement::RIGHT)
            Position += Right * velocity;
        if (direction == FlyCam_Movement::UP)
            Position += WorldUp * velocity;
        if (direction == FlyCam_Movement::DOWN)
            Position -= WorldUp * velocity;
    }

    void ProcessMouseLook(float xoffset, float yoffset, bool constrainPitch = true)
    {
        Yaw += xoffset * MouseSensitivity;
        Pitch += yoffset * MouseSensitivity;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};