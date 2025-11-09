// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include "Shader.h"
#include "ECS.h"
#include <glm/glm.hpp>
#include <memory>

class RenderSystem : public ecs::System {
public:
    RenderSystem() : m_QuadVAO(0) {}
    ~RenderSystem() {
        glDeleteVertexArrays(1, &m_QuadVAO);
    }

    void Init() {
        // --- Shader Setup (3D) ---
        const char* VS = R"glsl(
            #version 330 core
            layout (location = 0) in vec2 aPos; // Still a 2D quad

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main() {
                // We pass aPos.x, aPos.y for the quad, and 0.0 for z
                gl_Position = projection * view * model * vec4(aPos.x, aPos.y, 0.0, 1.0);
            }
        )glsl";

        const char* FS = R"glsl(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 spriteColor;
            void main() {
                FragColor = spriteColor;
            }
        )glsl";

        m_Shader.Compile(VS, FS);

        // --- VAO/VBO Setup for a simple quad ---
        float vertices[] = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.5f,  0.5f,

            -0.5f, -0.5f,
             0.5f,  0.5f,
            -0.5f,  0.5f
        };

        unsigned int VBO;
        glGenVertexArrays(1, &m_QuadVAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(m_QuadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Render(ecs::Registry* registry, const glm::mat4& projection, const glm::mat4& view) {
        m_Shader.Use();

        m_Shader.SetMat4("projection", projection);
        m_Shader.SetMat4("view", view);

        glBindVertexArray(m_QuadVAO);

        for (auto const& entity : m_Entities) {
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            auto& render = registry->GetComponent<RenderComponent>(entity);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, transform.position);

            // --- Make quads face the camera (billboarding) ---
            // We want them flat on the XZ plane, so we apply a 90-degree
            // rotation around the X-axis.
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            // Apply original rotation (which is now around Y-axis in world space)
            model = glm::rotate(model, transform.rotation, glm::vec3(0.0f, 1.0f, 0.0f));

            // Scale
            model = glm::scale(model, glm::vec3(transform.scale.x, transform.scale.y, 1.0f));

            m_Shader.SetMat4("model", model);
            m_Shader.SetVec4("spriteColor", render.color);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindVertexArray(0);
    }

private:
    Shader m_Shader;
    unsigned int m_QuadVAO;
};