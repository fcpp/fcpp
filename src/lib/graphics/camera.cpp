// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "lib/graphics/camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
: m_movementSpeed{ CAM_DEFAULT_SPEED },
  m_mouseSensitivity{ CAM_DEFAULT_SENSITIVITY },
  m_fov{ CAM_DEFAULT_FOV }
{
    setViewDefault(position, worldUp, yaw, pitch);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime)
{
    glm::mat4 trans;
    float velocity = m_movementSpeed * deltaTime;

    if (direction == FORWARD) {
        trans = glm::translate(glm::vec3(0.0f, 0.0f, velocity));
        m_view = trans * m_view;
    }
    if (direction == BACKWARD) {
        trans = glm::translate(glm::vec3(0.0f, 0.0f, -velocity));
        m_view = trans * m_view;
    }
    if (direction == LEFT) {
        trans = glm::translate(glm::vec3(velocity, 0.0f, 0.0f));
        m_view = trans * m_view;
    }
    if (direction == RIGHT) {
        trans = glm::translate(glm::vec3(-velocity, 0.0f, 0.0f));
        m_view = trans * m_view;
    }
    if (direction == FLY_UP) {
        trans = glm::translate(glm::vec3(0.0f, -velocity, 0.0f));
        m_view = trans * m_view;
    }
    if (direction == FLY_DOWN) {
        trans = glm::translate(glm::vec3(0.0f, velocity, 0.0f));
        m_view = trans * m_view;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset)
{
    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_view = glm::rotate(m_view, xoffset, glm::vec3(0.0f, 1.0f, 0.0f));
    m_view = glm::rotate(m_view, yoffset, glm::vec3(1.0f, 0.0f, 0.0f));
}

void Camera::processMouseScroll(float yoffset)
{
    m_fov -= (float)yoffset;

    if (m_fov < 1.0f)
        m_fov = 1.0f;
    if (m_fov > CAM_DEFAULT_FOV)
        m_fov = CAM_DEFAULT_FOV;
}

void Camera::applyViewDefault()
{
    m_view = m_viewDefault;
}

void Camera::setViewDefault(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
{
    // Calculate the front vector
    glm::vec3 calcFront;
    calcFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    calcFront.y = sin(glm::radians(pitch));
    calcFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    glm::vec3 front { glm::normalize(calcFront) };

    // Calculate the right
    glm::vec3 right { glm::normalize(glm::cross(front, worldUp)) };  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    
    // Calculate the up
    glm::vec3 up { glm::normalize(glm::cross(right, front)) };
    
    // Calculate view matrix with all the formerly obtained vectors
    m_viewDefault = m_view = glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getView()
{
    return m_view;
}

float Camera::getFov()
{
    return m_fov;
}
