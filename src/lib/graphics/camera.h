// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    FLY_UP,
    FLY_DOWN
};

// Default camera values
const float CAM_DEFAULT_YAW = -90.0f;
const float CAM_DEFAULT_PITCH = 0.0f;
const float CAM_DEFAULT_SPEED = 2.5f;
const float CAM_DEFAULT_SENSITIVITY = 0.1f;
const float CAM_DEFAULT_FOV = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
    // Camera vectors
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Camera angles (Euler Angles)
    float yaw;
    float pitch;

    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    float fov;

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();

public:
    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = CAM_DEFAULT_YAW, float pitch = CAM_DEFAULT_PITCH);

    // LookAt generator
    glm::mat4 getViewMatrix();

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void processKeyboard(CameraMovement direction, float deltaTime);

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void processMouseScroll(float yoffset);

    // Getters
    glm::vec3 getPosition();
    float getFov();
    float getYaw();
    float getPitch();
};
#endif