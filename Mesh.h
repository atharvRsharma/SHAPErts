#pragma once

#include <glad/glad.h>
#include "FileParser.h" 
#include <string>
#include <vector>
#include <cstddef>
#include <iostream> 

class Mesh
{
public:
    Mesh(const std::string& modelPath)
    {
        std::vector<Primitives::Vertex> vertices;
        std::vector<unsigned int> indices;
        
        // --- THIS IS THE FIX ---
        parseObj(modelPath, vertices, indices); // Was hard-coded

        if (vertices.empty()) {
            std::cerr << "ERROR::MESH: Failed to load model or model is empty: " << modelPath << std::endl;
            m_vertexCount = 0;
            m_indexCount = 0;
            m_vao = 0;
            m_vbo = 0;
            m_ebo = 0;
            return;
        }

        m_vertexCount = static_cast<unsigned int>(vertices.size());
        m_indexCount = static_cast<GLuint>(indices.size());

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Primitives::Vertex),
            vertices.data(),
            GL_STATIC_DRAW);


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(unsigned int) * indices.size(),
            indices.data(),
            GL_STATIC_DRAW);

        //pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Primitives::Vertex), (void*)offsetof(Primitives::Vertex, position));
        glEnableVertexAttribArray(0);
        //tex cooridnates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Primitives::Vertex), (void*)offsetof(Primitives::Vertex, texCoord));
        glEnableVertexAttribArray(1);
        //normals
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Primitives::Vertex), (void*)offsetof(Primitives::Vertex, normal));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
    }

    void bind() const {
        glBindVertexArray(m_vao);
    }

    void unbind() const {
        glBindVertexArray(0);
    }

    GLuint getVertexCount() const {
        return m_vertexCount;
    }
    GLuint getIndexCount() const {
        return m_indexCount;
    }
    GLuint getVAO() const {
        return m_vao;
    }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLuint m_vertexCount = 0;
    GLuint m_indexCount = 0;
};