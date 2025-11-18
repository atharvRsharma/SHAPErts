// All elements are generic placeholders for testing purposes.
#pragma once

#include "Shader.h"
#include "Components.h"
#include "ECS.h"
#include "InputSystem.h" 
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include "Mesh.h" 

class RenderSystem : public ecs::System {
public:
    int m_TotalCulled = 0;
    int m_TotalRendered = 0;

    RenderSystem() {}
    ~RenderSystem() {}

    void Init() {
        m_Shader.Compile("shaders/actor.vert", "shaders/actor.frag");
        m_GhostShader.Compile("shaders/highlighter.vert", "shaders/highlighter.frag");

        try {
            m_Meshes[MeshType::Quad] = std::make_shared<Mesh>("miscellaneous/plane.txt");
            m_Meshes[MeshType::Cube] = std::make_shared<Mesh>("miscellaneous/cube.txt");
            m_Meshes[MeshType::Base] = std::make_shared<Mesh>("miscellaneous/castle.obj");
            m_Meshes[MeshType::Turret] = std::make_shared<Mesh>("miscellaneous/finalTurret.obj");
            m_Meshes[MeshType::Sphere] = std::make_shared<Mesh>("miscellaneous/sphere.txt");
        }
        catch (const std::exception& e) {
            std::cerr << "mesh loading failure: " << e.what() << std::endl;
        }
    }

    void Render(ecs::Registry* registry, const glm::mat4& projection, const glm::mat4& view) {

        m_TotalCulled = 0;
        m_TotalRendered = 0;

        //opaque objs (actors)
        m_Shader.Use();
        m_Shader.SetMat4("projection", projection);
        m_Shader.SetMat4("view", view);
        glDepthMask(GL_TRUE);

        for (auto const& entity : m_Entities) {
            if (registry->HasComponent<GhostComponent>(entity)) continue;
            RenderEntity(registry, entity, m_Shader);
            m_TotalRendered++;
        }

        //ghost objects (highlighter)
        glDepthMask(GL_FALSE);
        m_GhostShader.Use();
        m_GhostShader.SetMat4("projection", projection);
        m_GhostShader.SetMat4("view", view);

        for (auto const& entity : registry->GetSystem<InputSystem>()->m_Entities) {
            RenderEntity(registry, entity, m_GhostShader);
        }
        glDepthMask(GL_TRUE);
    }

private:
    Shader m_Shader;
    Shader m_GhostShader;
    std::map<MeshType, std::shared_ptr<Mesh>> m_Meshes;

    void RenderEntity(ecs::Registry* registry, ecs::Entity entity, Shader& shader)
    {
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        auto& render = registry->GetComponent<RenderComponent>(entity);
        auto& meshComp = registry->GetComponent<MeshComponent>(entity);

        if (meshComp.type == MeshType::None) return;
        auto it = m_Meshes.find(meshComp.type);
        if (it == m_Meshes.end()) return;
        std::shared_ptr<Mesh> mesh = it->second;
        if (mesh->getVAO() == 0) return;

        mesh->bind();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, transform.position);
        model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 finalScale = transform.scale;
        if (meshComp.type == MeshType::Sphere) {
            finalScale *= 0.5f;
        }
        if (meshComp.type == MeshType::Turret) {
            finalScale *= 0.45f;
        }
        if (meshComp.type == MeshType::Base) {
            finalScale *= 0.15f;
        }
        model = glm::scale(model, finalScale);

        shader.SetMat4("model", model);
        shader.SetVec4("spriteColor", render.color);

        glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);

        mesh->unbind();
    }
};