#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
    : m_Position(position)
    , m_WorldUp(worldUp)
    , m_Yaw(yaw)
    , m_Pitch(pitch)
    , m_Front(0, 0, -1)
    , m_Up(worldUp)
    , m_Right(1, 0, 0)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::processKeyboard(CameraMovement dir, float deltaTime) {
    float velocity = moveSpeed * deltaTime;
    switch (dir) {
    case CameraMovement::FORWARD:  m_Position += m_Front * velocity; break;
    case CameraMovement::BACKWARD: m_Position -= m_Front * velocity; break;
    case CameraMovement::LEFT:     m_Position -= m_Right * velocity; break;
    case CameraMovement::RIGHT:    m_Position += m_Right * velocity; break;
    case CameraMovement::UP:       m_Position += m_WorldUp * velocity; break;
    case CameraMovement::DOWN:     m_Position -= m_WorldUp * velocity; break;
    }
}

void Camera::processMouse(float xOffset, float yOffset, bool constrainPitch) {
    m_Yaw += xOffset * mouseSens;
    m_Pitch += yOffset * mouseSens;

    // Prevent camera flip at the poles
    if (constrainPitch)
        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

    updateVectors();
}

void Camera::processScroll(float yOffset) {
    fov = std::clamp(fov - yOffset, 1.0f, 90.0f);
}

void Camera::updateVectors() {
    // Recalculate the front vector from Euler angles.
    // Spherical-to-Cartesian: this is the standard FPS-style derivation.
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
    front.y = std::sin(glm::radians(m_Pitch));
    front.z = std::sin(glm::radians(m_Yaw)) * std::cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    // Re-orthogonalize: right and up are derived from front + world-up
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}