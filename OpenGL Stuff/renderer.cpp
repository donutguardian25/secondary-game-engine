#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

// -----------------------------------------------------------------------
// Default shader — same Blinn-Phong as before, now owned by the Renderer
// -----------------------------------------------------------------------

static const char* VERT_SRC = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos     = vec3(worldPos);
    Normal      = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uProjection * uView * worldPos;
}
)";

static const char* FRAG_SRC = R"(
#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 uObjectColor;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform vec3 uViewPos;

void main() {
    float ambientStrength = 0.12;
    vec3 ambient = ambientStrength * uLightColor;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * uLightColor * uLightIntensity;

    vec3 viewDir    = normalize(uViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec      = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular   = 0.4 * spec * uLightColor;

    vec3 result = (ambient + diffuse + specular) * uObjectColor;
    FragColor   = vec4(result, 1.0);
}
)";

// -----------------------------------------------------------------------

Renderer::Renderer(int viewportWidth, int viewportHeight)
    : m_Width(viewportWidth)
    , m_Height(viewportHeight)
    , m_AspectRatio((float)viewportWidth / (float)viewportHeight)
    , m_View(1.0f)
    , m_Projection(1.0f)
{
    m_Shader = std::make_unique<Shader>(VERT_SRC, FRAG_SRC);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);   // skip back faces — free ~50% of fragment work
    glCullFace(GL_BACK);
}

void Renderer::onResize(int width, int height) {
    m_Width = width;
    m_Height = height;
    m_AspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
}

void Renderer::beginFrame(const Camera& camera) {
    // Reset stats
    m_Stats = {};

    // Cache matrices — computed once per frame, not once per draw call
    m_View = camera.getViewMatrix();
    m_Projection = camera.getProjectionMatrix(m_AspectRatio);

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Shader->bind();
    uploadCameraUniforms();

    // Upload light uniforms once per frame (shared by all draw calls)
    m_Shader->setVec3("uLightPos", light.position);
    m_Shader->setVec3("uLightColor", light.color);
    m_Shader->setFloat("uLightIntensity", light.intensity);
    m_Shader->setVec3("uViewPos", camera.position());
}

void Renderer::submit(const Mesh& mesh,
    const glm::mat4& transform,
    const glm::vec3& color)
{
    m_Queue.push_back({ &mesh, transform, color });
}

void Renderer::endFrame() {
    // Walk the queue and issue one draw call per command.
    // Future extension point: sort by material/shader to reduce state changes,
    // or batch by mesh type for instanced rendering.
    for (const auto& cmd : m_Queue)
        issueDrawCall(cmd);

    m_Queue.clear();
    m_Shader->unbind();
}

void Renderer::drawImmediate(const Mesh& mesh,
    const glm::mat4& transform,
    const glm::vec3& color)
{
    issueDrawCall({ &mesh, transform, color });
}

void Renderer::uploadCameraUniforms() {
    m_Shader->setMat4("uView", m_View);
    m_Shader->setMat4("uProjection", m_Projection);
}

void Renderer::issueDrawCall(const RenderCommand& cmd) {
    m_Shader->setMat4("uModel", cmd.transform);
    m_Shader->setVec3("uObjectColor", cmd.color);

    cmd.mesh->draw();

    ++m_Stats.drawCalls;
    m_Stats.triangles += cmd.mesh->indexCount() / 3;
}