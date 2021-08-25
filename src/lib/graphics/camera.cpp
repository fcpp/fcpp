// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <algorithm>

#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "lib/graphics/camera.hpp"

// using namespace fcpp to prevent very verbose code...
using namespace fcpp;


/* --- CONSTRUCTOR --- */
camera::camera()
: m_mouseSensitivity{ CAM_DEFAULT_SENSITIVITY },
  m_depth{ CAM_DEFAULT_DEPTH },
  m_depthDefault{ CAM_DEFAULT_DEPTH },
  m_diagonal{ 1000 },
  m_aspectRatio{ 4.0f / 3.0f  },
  m_view{ 1.0f },
  m_viewDefault{ 1.0f },
  m_perspective{ 1.0f },
  m_ortho{ 1.0f } {}


/* --- PUBLIC FUNCTIONS --- */
void camera::setViewDefault(glm::vec3 position, float depth, glm::vec3 worldUp, float yaw, float pitch)
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

    // Sets the default depth
    m_depthDefault = m_depth = depth;
    updatePerspective();
}

void camera::applyViewDefault() {
    m_view = m_viewDefault;
    m_depth = m_depthDefault;
    updatePerspective();
}

glm::mat4 const& camera::getView() const {
    return m_view;
}

glm::mat4 const& camera::getPerspective() const {
    return m_perspective;
}

glm::mat4 const& camera::getOrthographic() const {
    return m_ortho;
}

glm::vec3 camera::getPosition() const {
    glm::vec4 col{ glm::column(glm::affineInverse(m_view), 3) };
    return glm::vec3{ col.x, col.y, col.z };
}

float camera::getDepth() const {
    return m_depth;
}

void camera::setScreen(float width, float height) {
    m_diagonal = std::sqrt(width*width + height*height) / 2;
    m_aspectRatio = width / height;
    updatePerspective();
    updateOrthographic(width, height);
}

void camera::mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods)
{
    switch (type) {
        case mouse_type::scroll:
        {
            float new_depth = m_depth * pow(0.98, (mods & GLFW_MOD_SHIFT) > 0 ? y/10 : y);
            m_view = glm::translate(glm::vec3(0.0f, 0.0f, m_depth-new_depth)) * m_view;
            m_depth = new_depth;
            updatePerspective();
            break;
        }
        case mouse_type::drag:
        {
            float a = (xFirst*x + yFirst*y) / m_diagonal;
            float b = (xFirst*y - yFirst*x) / m_diagonal;
            if (std::abs(a) < CAM_THRESHOLD * std::max(std::abs(b), 1.0f)) a = 0;
            if (std::abs(b) < CAM_THRESHOLD * std::max(std::abs(a), 1.0f)) b = 0;
            a *= m_mouseSensitivity;
            b *= m_mouseSensitivity;
            if ((mods & GLFW_MOD_SHIFT) > 0) {
                a /= 10;
                b /= 10;
            }

            // to add translation with rotation
            m_view =
                glm::translate(glm::normalize(glm::vec3(xFirst, yFirst, 0.0f))*a*m_depth*0.02f) *
                glm::rotate(glm::radians(-a), glm::vec3{yFirst, -xFirst, 0.0f}) *
                glm::rotate(glm::radians(b), glm::vec3{0.0f, 0.0f, 1.0f}) * m_view;
            break;
        }
        default:
            break;
    }
}

bool camera::keyboardInput(int key, bool first, float deltaTime, int mods)
{
    float velocity = m_depth * deltaTime * ((mods & GLFW_MOD_SHIFT) > 0 ? 0.05 : 0.5);
    switch (key) {
        case GLFW_KEY_E:
            m_view = glm::translate(glm::vec3(0.0f, 0.0f, velocity)) * m_view;
            break;
        case GLFW_KEY_Q:
            m_view = glm::translate(glm::vec3(0.0f, 0.0f, -velocity)) * m_view;
            break;
        case GLFW_KEY_A:
            m_view = glm::translate(glm::vec3(velocity, 0.0f, 0.0f)) * m_view;
            break;
        case GLFW_KEY_D:
            m_view = glm::translate(glm::vec3(-velocity, 0.0f, 0.0f)) * m_view;
            break;
        case GLFW_KEY_W:
            m_view = glm::translate(glm::vec3(0.0f, -velocity, 0.0f)) * m_view;
            break;
        case GLFW_KEY_S:
            m_view = glm::translate(glm::vec3(0.0f, velocity, 0.0f)) * m_view;
            break;
        case GLFW_KEY_C:
            if (first) applyViewDefault();
            break;
        default:
            return false;
    }
    return true;
}


/* --- PRIVATE METHODS --- */
void camera::updatePerspective() {
    m_perspective = glm::perspective(glm::radians(CAM_DEFAULT_FOV), m_aspectRatio, m_depth / 32, m_depth * 32);
}

void camera::updateOrthographic(float width, float height) {
    m_ortho = glm::ortho(0.0f, width, 0.0f, height);
}
