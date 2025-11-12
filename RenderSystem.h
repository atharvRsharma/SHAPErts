// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include "Shader.h"
#include "Components.h"
#include "ECS.h"
#include <glm/glm.hpp>
#include <memory>
#include <map>

class RenderSystem : public ecs::System {
public:
    RenderSystem() {} // No more m_QuadVAO
    ~RenderSystem() {
        // Clean up all VAOs and VBOs
        for (auto const& [type, vao] : m_VAOs) {
            glDeleteVertexArrays(1, &vao);
        }
        for (auto const& [type, vbo] : m_VBOs) {
            glDeleteBuffers(1, &vbo);
        }
        for (auto const& [type, ebo] : m_EBOs) {
            glDeleteBuffers(1, &ebo);
        }
    }

    void Init() {
        // --- Shader Setup (3D) ---
        // This shader will work for any 3D, non-textured mesh
        const char* VS = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 aPos; // --- NOW VEC3 ---

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
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

        // --- NOW WE CREATE ALL OUR MESHES ---
        CreateQuadMesh();
        CreateCubeMesh();
        // CreateUnitSoldierMesh(); // We could add this
    }
    void Render(ecs::Registry* registry, const glm::mat4& projection, const glm::mat4& view) {
        m_Shader.Use();

        m_Shader.SetMat4("projection", projection);
        m_Shader.SetMat4("view", view);

        // --- NEW RENDER LOOP ---
        for (auto const& entity : m_Entities) {
            // Get all components needed for rendering
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            auto& render = registry->GetComponent<RenderComponent>(entity);
            auto& mesh = registry->GetComponent<MeshComponent>(entity);

            // Skip entities that have no mesh
            if (mesh.type == MeshType::None) continue;

            // Look up the VAO and index count for this mesh type
            GLuint vao = m_VAOs[mesh.type];
            GLsizei indexCount = m_IndexCounts[mesh.type];

            glBindVertexArray(vao);

            // --- HIERARCHICAL RENDERING FOR SOLDIER ---
            if (mesh.type == MeshType::Unit_Soldier) {
                // If it's a soldier, we do two draws

                // 1. Draw Body (a cuboid)
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, transform.position);
                model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f)); // 0.5 x 1.0 x 0.5 cuboid

                m_Shader.SetMat4("model", model);
                m_Shader.SetVec4("spriteColor", render.color);
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

                // 2. Draw Head (a cube)
                // We offset it from the *base* transform, not the scaled one
                glm::vec3 headPos = transform.position + glm::vec3(0.0f, 0.75f, 0.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, headPos);
                model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));

                m_Shader.SetMat4("model", model);
                m_Shader.SetVec4("spriteColor", { 1.0f, 0.8f, 0.6f, 1.0f }); // Skin tone
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

            }
            else {
                // --- STANDARD RENDERING (Quad, Cube, etc.) ---
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, transform.position);
                // Apply Euler angle rotations
                model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, transform.scale);

                m_Shader.SetMat4("model", model);
                m_Shader.SetVec4("spriteColor", render.color);

                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
            }
        }

        glBindVertexArray(0);
    }

private:
    Shader m_Shader;

    // --- MESH STORAGE ---
    std::map<MeshType, GLuint> m_VAOs;
    std::map<MeshType, GLuint> m_VBOs;
    std::map<MeshType, GLuint> m_EBOs; // Element Buffer Objects (for indices)
    std::map<MeshType, GLsizei> m_IndexCounts;

    // --- MESH CREATION FUNCTIONS ---

    void CreateQuadMesh() {
        // A quad on the XZ plane (for our grid)
        float quadVertices[] = {
            // positions
             0.5f,  0.0f,  0.5f,
             0.5f,  0.0f, -0.5f,
            -0.5f,  0.0f, -0.5f,
            -0.5f,  0.0f,  0.5f
        };
        unsigned int quadIndices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

        GLuint VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        // Store references
        m_VAOs[MeshType::Quad] = VAO;
        m_VBOs[MeshType::Quad] = VBO;
        m_EBOs[MeshType::Quad] = EBO;
        m_IndexCounts[MeshType::Quad] = 6;
    }

    void CreateCubeMesh() {
        // A standard 1x1x1 cube centered at (0,0,0)
        float cubeVertices[] = {
            // positions
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f
        };
        unsigned int cubeIndices[] = {
            0, 1, 2, 2, 3, 0, // Back face
            4, 5, 6, 6, 7, 4, // Front face
            0, 4, 7, 7, 3, 0, // Left face
            1, 5, 6, 6, 2, 1, // Right face
            3, 2, 6, 6, 7, 3, // Top face
            0, 1, 5, 5, 4, 0  // Bottom face
        };

        GLuint VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        // Store references
        m_VAOs[MeshType::Cube] = VAO;
        m_VBOs[MeshType::Cube] = VBO;
        m_EBOs[MeshType::Cube] = EBO;
        m_IndexCounts[MeshType::Cube] = 36;

        // --- HACK: Map Soldier to the Cube Mesh ---
        // This tells the soldier to use the cube's VAO
        m_VAOs[MeshType::Unit_Soldier] = VAO;
        m_IndexCounts[MeshType::Unit_Soldier] = 36;
    }
};