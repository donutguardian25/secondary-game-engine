#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexSrc, const char* fragmentSrc);
    ~Shader();

    void bind() const;
    void unbind() const;

    // Uniform setters
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    mutable std::unordered_map<std::string, int> m_UniformCache;

    int getUniformLocation(const std::string& name) const;
    static unsigned int compileShader(unsigned int type, const char* src);
};