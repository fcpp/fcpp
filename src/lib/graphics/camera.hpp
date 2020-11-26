// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

//! @brief Enum defining several possible options for camera movement, abstracting from specific input methods.
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    FLY_UP,
    FLY_DOWN
};

//! @brief Default camera's yaw
const float CAM_DEFAULT_YAW{ -90.0f };

//! @brief Default camera's pitch
const float CAM_DEFAULT_PITCH{ 0.0f };

//! @brief Default camera's speed
const float CAM_DEFAULT_SPEED{ 50.0f };

//! @brief Default camera's sensitivity
const float CAM_DEFAULT_SENSITIVITY{ 0.1f };

//! @brief Default camera's Field of View
const float CAM_DEFAULT_FOV{ 45.0f };


//! @brief Camera class with integrated view matrix.
class Camera
{
public:
    //! @brief Camera's constructor, with default values for the camera's initial vectors.
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = CAM_DEFAULT_YAW, float pitch = CAM_DEFAULT_PITCH);

    //! @brief It processes input received from keyboard.
    void processKeyboard(CameraMovement direction, float deltaTime);

    //! @brief It processes input received from mouse input.
    void processMouseMovement(float xoffset, float yoffset);

    //! @brief It processes input received from a mouse scroll-wheel.
    void processMouseScroll(float yoffset);
    
    //! @brief It moves camera to its initial position and rotation.
    void applyViewDefault();
    
    //! @brief It sets camera's default view matrix; it also sets the current view matrix to these values.
    void setViewDefault(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch);

    //! @brief It returns camera's view matrix.
    glm::mat4 getView();
    
    //! @brief It returns camera's Field of View.
    float getFov();

private:    
    //! @brief Camera's current movement speed.
    float m_movementSpeed;
    
    //! @brief Camera's current mouse sensitivity.
    float m_mouseSensitivity;
    
    //! @brief Camera's current Field of View.
    float m_fov;
    
    //! @brief Camera's view matrix; all trasnformations are made on this one.
    glm::mat4 m_view;
    
    //! @brief Camera's default view matrix; it is stored to easily return to the default position.
    glm::mat4 m_viewDefault;
};
#endif
