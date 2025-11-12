// All elements are generic placeholders for testing purposes.
#pragma once

#include "Systems.h"
#include "Shader.h"
#include "Components.h"
#include "ECS.h"
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include "Mesh.h" // Include your new Mesh class

class RenderSystem : public ecs::System {
public:
    RenderSystem() {}
    ~RenderSystem() {}

    void Init() {
        // --- Shader Setup ---
        // We update the shader to accept all Vertex attributes
        const char* VS = R"glsl(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aTexCoord;
            layout (location = 2) in vec3 aNormal;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            
            out vec2 TexCoord;
            out vec3 Normal;
            out vec3 FragPos;

            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
                FragPos = vec3(model * vec4(aPos, 1.0));
                Normal = mat3(transpose(inverse(model))) * aNormal;
                TexCoord = aTexCoord;
            }
        )glsl";

        const char* FS = R"glsl(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 spriteColor;
            
            in vec2 TexCoord;
            in vec3 Normal;
            in vec3 FragPos;

            void main() {
                // For now, just use the color.
                FragColor = spriteColor;
            }
        )glsl";

        m_Shader.Compile(VS, FS);

        try {
            m_Meshes[MeshType::Quad] = std::make_shared<Mesh>("plane.txt");
            //m_Meshes[MeshType::Cube] = std::make_shared<Mesh>("cube.txt");
            m_Meshes[MeshType::Pyramid] = std::make_shared<Mesh>("pyr - Copy.obj");
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to load meshes: " << e.what() << std::endl;
        }
    }

    void Render(ecs::Registry* registry, const glm::mat4& projection, const glm::mat4& view) {
        m_Shader.Use();
        m_Shader.SetMat4("projection", projection);
        m_Shader.SetMat4("view", view);

        for (auto const& entity : m_Entities) {
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            auto& render = registry->GetComponent<RenderComponent>(entity);
            auto& meshComp = registry->GetComponent<MeshComponent>(entity);

            if (meshComp.type == MeshType::None) continue;

            // Find the mesh
            auto it = m_Meshes.find(meshComp.type);
            if (it == m_Meshes.end()) continue; // No mesh loaded for this type

            std::shared_ptr<Mesh> mesh = it->second;
            if (mesh->getVAO() == 0) continue; // Check if mesh failed to load

            // Bind the mesh's VAO
            mesh->bind();

            // Set model matrix
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, transform.position);
            model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, transform.scale);

            m_Shader.SetMat4("model", model);
            m_Shader.SetVec4("spriteColor", render.color);

            // Draw using the mesh's index count
            glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);

            // Unbind
            mesh->unbind();
        }
    }

private:
    Shader m_Shader;

    // --- NEW MESH STORAGE ---
    // We just store your Mesh objects directly
    std::map<MeshType, std::shared_ptr<Mesh>> m_Meshes;
};