// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file camera.hpp
 * @brief Implementation of the `camera` class.
 */

#ifndef FCPP_GRAPHICS_CAMERA_H
#define FCPP_GRAPHICS_CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


namespace fcpp {

//! @brief Supported mouse input type.
enum class mouse_type : char { hover, click, drag, scroll };

//! @brief Default camera's depth
constexpr float CAM_DEFAULT_DEPTH{ 10.0f };

//! @brief Default camera's yaw
constexpr float CAM_DEFAULT_YAW{ -90.0f };

    //! @brief Default camera's pitch
constexpr float CAM_DEFAULT_PITCH{ 0.0f };

    //! @brief Default camera's speed
constexpr float CAM_DEFAULT_SPEED{ 50.0f };

    //! @brief Default camera's sensitivity
constexpr float CAM_DEFAULT_SENSITIVITY{ 0.3f };

    //! @brief Threshold for unintentional camera movement
constexpr float CAM_THRESHOLD{ 0.7f };

    //! @brief Default camera's Field of View
constexpr float CAM_DEFAULT_FOV{ 45.0f };


//! @brief Camera class with integrated view matrix.
class camera {
  public:
    //! @brief Camera's constructor, with default values for the camera's initial vectors.
    camera();

    //! @brief It sets camera's default and current view and projection matrix.
    void setViewDefault(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float depth = CAM_DEFAULT_DEPTH, glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = CAM_DEFAULT_YAW, float pitch = CAM_DEFAULT_PITCH);

    //! @brief It moves camera to its initial position and rotation.
    void applyViewDefault();

    //! @brief It returns camera's view matrix.
    glm::mat4 const& getView() const;

    //! @brief It returns camera's perspective matrix.
    glm::mat4 const& getPerspective() const;

    //! @brief It returns camera's orthographic matrix.
    glm::mat4 const& getOrthographic() const;

    //! @brief It returns the world space position of the camera.
    glm::vec3 getPosition() const;

    //! @brief It returns the camera's current view plane (proportional to zNear, zFar and movement speed).
    float getDepth() const;

    //! @brief Sets the current screen's height, width and diagonal size given window size.
    void setScreen(float width, float height);

    //! @brief It manages mouse input of the given type.
    void mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods);

    //! @brief Given the key stroke, the press status and a deltaTime, it manages keyboard input for the camera.
    bool keyboardInput(int key, bool first, float deltaTime, int mods);

private:
    //! @brief Camera's current mouse sensitivity.
    float m_mouseSensitivity;

    //! @brief Camera's current view plane (proportional to zNear, zFar and movement speed).
    float m_depth;

    //! @brief Camera's default view plane (proportional to zNear, zFar and movement speed).
    float m_depthDefault;

    //! @brief The screen diagonal.
    float m_diagonal;

    //! @brief The screen aspect ratio.
    float m_aspectRatio;

    //! @brief Camera's view matrix; all transformations are made on this one.
    glm::mat4 m_view;

    //! @brief Camera's default view matrix.
    glm::mat4 m_viewDefault;

    //! @brief Camera's perspective matrix.
    glm::mat4 m_perspective;

    //! @brief Camera's orthographic matrix.
    glm::mat4 m_ortho;

    //! @brief Updates the perspective matrix.
    void updatePerspective();

    //! @brief Updates the orthographic matrix, given the width and height of the window.
    void updateOrthographic(float width, float height);
};


}

#endif // FCPP_GRAPHICS_CAMERA_H
