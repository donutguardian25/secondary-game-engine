#pragma once
#include <glm/glm.hpp>

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
public:
    // Projection settings
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    // Look sensitivity / movement speed
    float moveSpeed = 5.0f;
    float mouseSens = 0.1f;

    Camera(glm::vec3 position = { 0, 0, 3 },
        glm::vec3 worldUp = { 0, 1, 0 },
        float yaw = -90.0f,
        float pitch = 0.0f);

    glm::mat4 getViewMatrix()                          const;
    glm::mat4 getProjectionMatrix(float aspectRatio)   const;

    // Per-frame input handlers — call from your main loop
    void processKeyboard(CameraMovement dir, float deltaTime);
    void processMouse(float xOffset, float yOffset, bool constrainPitch = true);
    void processScroll(float yOffset);

    glm::vec3 position() const { return m_Position; }
    glm::vec3 front()    const { return m_Front; }

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;

    void updateVectors();
};