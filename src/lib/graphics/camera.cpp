// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "lib/graphics/camera.hpp"
#include "lib/graphics/input_types.hpp"

// using namespace fcpp to prevent very verbose code...
using namespace fcpp;


Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
: m_movementSpeed{ CAM_DEFAULT_SPEED },
  m_mouseSensitivity{ CAM_DEFAULT_SENSITIVITY },
  m_fov{ CAM_DEFAULT_FOV }
{
    setViewDefault(position, worldUp, yaw, pitch);
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

glm::mat4 const& Camera::getView() const
{
    return m_view;
}

float Camera::getFov() const
{
    return m_fov;
}

void Camera::mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type)
{
    switch (type) {
        case mouse_type::scroll:
            m_fov -= (float)y;

            if (m_fov < 1.0f)
                m_fov = 1.0f;
            if (m_fov > CAM_DEFAULT_FOV)
                m_fov = CAM_DEFAULT_FOV;
            break;
        case mouse_type::fpp:
            glm::mat4 rot;
            x *= m_mouseSensitivity;
            y *= m_mouseSensitivity;

            rot = glm::rotate((float)glm::radians(x), glm::vec3{0.0f, 1.0f, 0.0f});
            m_view = rot * m_view;
            rot = glm::rotate((float)glm::radians(-y), glm::vec3{1.0f, 0.0f, 0.0f});
            m_view = rot * m_view;
            break;
        case mouse_type::drag:
            float a = (xFirst*x + yFirst*y) / m_diagonal;
            float b = (xFirst*y - yFirst*x) / m_diagonal;
	        if (std::abs(a) < CAM_THRESHOLD * std::max(std::abs(b), 1.0f)) a = 0;
	        if (std::abs(b) < CAM_THRESHOLD * std::max(std::abs(a), 1.0f)) b = 0;

            m_view =
		        glm::rotate(glm::radians(a * m_mouseSensitivity), glm::vec3{yFirst, -xFirst, 0.0f}) *
		        glm::rotate(glm::radians(b * m_mouseSensitivity), glm::vec3{0.0f, 0.0f, 1.0f}) * m_view;
            break;
    }
}

void Camera::keyboardInput(int key, bool first, float deltaTime)
{
    /*if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm::mat4 trans;
        float velocity = m_movementSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            trans = glm::translate(glm::vec3(0.0f, 0.0f, velocity));
            m_view = trans * m_view;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            trans = glm::translate(glm::vec3(0.0f, 0.0f, -velocity));
            m_view = trans * m_view;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            trans = glm::translate(glm::vec3(velocity, 0.0f, 0.0f));
            m_view = trans * m_view;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            trans = glm::translate(glm::vec3(-velocity, 0.0f, 0.0f));
            m_view = trans * m_view;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            trans = glm::translate(glm::vec3(0.0f, -velocity, 0.0f));
            m_view = trans * m_view;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            trans = glm::translate(glm::vec3(0.0f, velocity, 0.0f));
            m_view = trans * m_view;
        }
    }*/
}
