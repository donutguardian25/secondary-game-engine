#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Camera.h"
#include "Mesh.h"
#include "Renderer.h"

static Camera  g_Camera({ 0.0f, 1.5f, 6.0f });
static float   g_LastX = 640.0f, g_LastY = 360.0f;
static bool    g_FirstMouse = true;
static bool    g_MouseCaptured = false;

static Mesh makeCube() {
    std::vector<Vertex> verts = {
        {{-0.5f,-0.5f, 0.5f}, { 0, 0, 1}, {0,0}}, {{ 0.5f,-0.5f, 0.5f}, { 0, 0, 1}, {1,0}},
        {{ 0.5f, 0.5f, 0.5f}, { 0, 0, 1}, {1,1}}, {{-0.5f, 0.5f, 0.5f}, { 0, 0, 1}, {0,1}},
        {{ 0.5f,-0.5f,-0.5f}, { 0, 0,-1}, {0,0}}, {{-0.5f,-0.5f,-0.5f}, { 0, 0,-1}, {1,0}},
        {{-0.5f, 0.5f,-0.5f}, { 0, 0,-1}, {1,1}}, {{ 0.5f, 0.5f,-0.5f}, { 0, 0,-1}, {0,1}},
        {{-0.5f,-0.5f,-0.5f}, {-1, 0, 0}, {0,0}}, {{-0.5f,-0.5f, 0.5f}, {-1, 0, 0}, {1,0}},
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1,1}}, {{-0.5f, 0.5f,-0.5f}, {-1, 0, 0}, {0,1}},
        {{ 0.5f,-0.5f, 0.5f}, { 1, 0, 0}, {0,0}}, {{ 0.5f,-0.5f,-0.5f}, { 1, 0, 0}, {1,0}},
        {{ 0.5f, 0.5f,-0.5f}, { 1, 0, 0}, {1,1}}, {{ 0.5f, 0.5f, 0.5f}, { 1, 0, 0}, {0,1}},
        {{-0.5f, 0.5f, 0.5f}, { 0, 1, 0}, {0,0}}, {{ 0.5f, 0.5f, 0.5f}, { 0, 1, 0}, {1,0}},
        {{ 0.5f, 0.5f,-0.5f}, { 0, 1, 0}, {1,1}}, {{-0.5f, 0.5f,-0.5f}, { 0, 1, 0}, {0,1}},
        {{-0.5f,-0.5f,-0.5f}, { 0,-1, 0}, {0,0}}, {{ 0.5f,-0.5f,-0.5f}, { 0,-1, 0}, {1,0}},
        {{ 0.5f,-0.5f, 0.5f}, { 0,-1, 0}, {1,1}}, {{-0.5f,-0.5f, 0.5f}, { 0,-1, 0}, {0,1}},
    };
    std::vector<unsigned int> indices;
    for (unsigned int f = 0; f < 6; ++f) {
        unsigned int b = f * 4;
        indices.insert(indices.end(), { b, b + 1, b + 2, b + 2, b + 3, b });
    }
    return Mesh(verts, indices);
}

static int g_Width = 1280, g_Height = 720;
static Renderer* g_RendererPtr = nullptr;

static void framebufferSizeCallback(GLFWwindow*, int w, int h) {
    g_Width = w; g_Height = h;
    if (g_RendererPtr) g_RendererPtr->onResize(w, h);
}

static void mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (!g_MouseCaptured) return;
    if (g_FirstMouse) { g_LastX = (float)xpos; g_LastY = (float)ypos; g_FirstMouse = false; }
    float xOff = (float)xpos - g_LastX;
    float yOff = g_LastY - (float)ypos;
    g_LastX = (float)xpos; g_LastY = (float)ypos;
    g_Camera.processMouse(xOff, yOff);
}

static void scrollCallback(GLFWwindow*, double, double y) { g_Camera.processScroll((float)y); }

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        g_MouseCaptured = !g_MouseCaptured;
        g_FirstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR,
            g_MouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

static void processMovement(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) g_Camera.processKeyboard(CameraMovement::FORWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) g_Camera.processKeyboard(CameraMovement::BACKWARD, dt);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) g_Camera.processKeyboard(CameraMovement::LEFT, dt);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) g_Camera.processKeyboard(CameraMovement::RIGHT, dt);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) g_Camera.processKeyboard(CameraMovement::UP, dt);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) g_Camera.processKeyboard(CameraMovement::DOWN, dt);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(g_Width, g_Height, "Engine Demo", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    Renderer renderer(g_Width, g_Height);
    g_RendererPtr = &renderer;
    renderer.light = { {3.0f, 5.0f, 3.0f}, {1.0f, 1.0f, 0.95f}, 1.2f };

    Mesh cube = makeCube();

    struct SceneObject { glm::vec3 pos, scale, color; float rotSpeed; };
    std::vector<SceneObject> scene = {
        {{ 0.0f,  0.0f,  0.0f}, {1.0f,1.0f,1.0f}, {0.40f,0.70f,1.00f}, 0.6f},
        {{ 2.5f,  0.0f, -1.0f}, {0.7f,0.7f,0.7f}, {1.00f,0.55f,0.35f}, 1.0f},
        {{-2.5f,  0.5f,  0.5f}, {1.2f,1.2f,1.2f}, {0.45f,0.85f,0.55f}, 0.4f},
        {{ 0.5f, -0.8f,  2.5f}, {0.5f,0.5f,0.5f}, {0.90f,0.80f,0.30f}, 1.5f},
        {{-1.0f,  1.5f, -2.0f}, {0.8f,0.8f,0.8f}, {0.75f,0.45f,0.90f}, 0.8f},
    };

    float lastTime = 0.0f, statTimer = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        processMovement(window, dt);

        renderer.beginFrame(g_Camera);

        for (const auto& obj : scene) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), obj.pos);
            model = glm::rotate(model, now * obj.rotSpeed, glm::vec3(0.5f, 1.0f, 0.3f));
            model = glm::scale(model, obj.scale);
            renderer.submit(cube, model, obj.color);
        }

        renderer.endFrame();

        // Print frame stats once per second
        statTimer += dt;
        if (statTimer >= 1.0f) {
            const auto& s = renderer.stats();
            std::cout << "Draw calls: " << s.drawCalls
                << "  Triangles: " << s.triangles
                << "  FPS: " << (int)(1.0f / dt) << "\n";
            statTimer = 0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}