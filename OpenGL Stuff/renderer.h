#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"

// -----------------------------------------------------------------------
// A draw command: everything needed to render one object.
// The Renderer accumulates these during the frame, then flushes them.
// -----------------------------------------------------------------------
struct RenderCommand {
    const Mesh* mesh;
    glm::mat4     transform;   // model matrix
    glm::vec3     color;
};

// -----------------------------------------------------------------------
// Light description (point light for now)
// -----------------------------------------------------------------------
struct PointLight {
    glm::vec3 position = { 2.0f, 4.0f, 2.0f };
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float     intensity = 1.0f;
};

// -----------------------------------------------------------------------
// Renderer
// Owns the default shader and manages the frame lifecycle.
// -----------------------------------------------------------------------
class Renderer {
public:
    explicit Renderer(int viewportWidth, int viewportHeight);

    // Call at the start of each frame
    void beginFrame(const Camera& camera);

    // Queue a mesh for drawing this frame
    void submit(const Mesh& mesh,
        const glm::mat4& transform,
        const glm::vec3& color = { 1, 1, 1 });

    // Flush the command queue, issue draw calls, and clear for next frame
    void endFrame();

    // Convenience: draw a single object immediately (no queuing)
    void drawImmediate(const Mesh& mesh,
        const glm::mat4& transform,
        const glm::vec3& color = { 1, 1, 1 });

    // Scene light (one point light for now; easy to extend to an array)
    PointLight light;

    void onResize(int width, int height);

    // Stats — reset each frame in beginFrame()
    struct Stats {
        unsigned int drawCalls = 0;
        unsigned int triangles = 0;
    };
    const Stats& stats() const { return m_Stats; }

private:
    std::unique_ptr<Shader> m_Shader;

    int   m_Width, m_Height;
    float m_AspectRatio;

    // Cached per-frame matrices from the camera
    glm::mat4 m_View;
    glm::mat4 m_Projection;

    std::vector<RenderCommand> m_Queue;

    Stats m_Stats;

    void uploadCameraUniforms();
    void issueDrawCall(const RenderCommand& cmd);
};