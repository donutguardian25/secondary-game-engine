#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
public:
    // Upload geometry to the GPU on construction
    Mesh(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);
    ~Mesh();

    // Non-copyable: owns GPU resources
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Move is fine
    Mesh(Mesh&& other) noexcept;

    void draw() const;

    unsigned int indexCount() const { return m_IndexCount; }

private:
    unsigned int m_VAO, m_VBO, m_EBO;
    unsigned int m_IndexCount;

    void setupMesh(const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);
};
