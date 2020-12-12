// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "lib/graphics/input_types.hpp"


namespace fcpp {
    //! @brief Default camera's yaw
    const float CAM_DEFAULT_YAW{ -90.0f };

    //! @brief Default camera's pitch
    const float CAM_DEFAULT_PITCH{ 0.0f };

    //! @brief Default camera's speed
    const float CAM_DEFAULT_SPEED{ 50.0f };

    //! @brief Default camera's sensitivity
    const float CAM_DEFAULT_SENSITIVITY{ 0.1f };

    //! @brief Threshold for unintentional camera movement
    const float CAM_THRESHOLD{ 0.7f };

    //! @brief Default camera's Field of View
    const float CAM_DEFAULT_FOV{ 45.0f };


    //! @brief Camera class with integrated view matrix.
    class Camera
    {
    public:
        //! @brief Camera's constructor, with default values for the camera's initial vectors.
        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = CAM_DEFAULT_YAW, float pitch = CAM_DEFAULT_PITCH);
        
        //! @brief It moves camera to its initial position and rotation.
        void applyViewDefault();
        
        //! @brief It sets camera's default view matrix; it also sets the current view matrix to these values.
        void setViewDefault(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch);

        //! @brief It returns camera's view matrix.
        glm::mat4 getView();
        
        //! @brief It returns camera's Field of View.
        float getFov();

	    //! @brief Sets the current screen diagonal size given window size.
	    void setDiagonal(float width, float height) {
		    m_diagonal = std::sqrt(width*width + height*height) / 2;
	    }
	    
	    //! @brief It manages mouse input of the given type.
        void mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type);
        
        //! @brief Given a deltaTime, it manages keyboard input for the camera.
        void keyboardInput(GLFWwindow* window, float deltaTime);

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

	    //! @brief The screen diagonal.
	    float m_diagonal;
    };
}
#endif
