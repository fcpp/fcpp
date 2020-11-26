// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_RENDERER_H_
#define FCPP_GRAPHICS_RENDERER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

#include "lib/graphics/camera.hpp"
#include "lib/graphics/shader.hpp"
#include "lib/graphics/shapes.hpp"


namespace fcpp {
    //! @brief Color type as a packed integer, for usage in template parameters.
    using packed_color = uint32_t;

    //! @brief Colors for representing nodes.
    struct color {
        //! @brief Default color (black).
        color() : rgba{ 0,0,0,1 } {}

        //! @brief Color constructor from RGBA values.
        template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        color(T r, T g, T b, T a = 1) {
            float mx = std::is_integral<T>::value ? 255 : 1;
            rgba[0] = r / mx;
            rgba[1] = g / mx;
            rgba[2] = b / mx;
            rgba[3] = a / mx;
        }

        //! @brief Color constructor from a packed integral RGBA value.
        color(packed_color irgba) : color((irgba >> 24) & 255, (irgba >> 16) & 255, (irgba >> 8) & 255, irgba & 255) {}

        //! @brief Access to the red component.
        float& red() {
            return rgba[0];
        }

        //! @brief Const access to the red component.
        float const& red() const {
            return rgba[0];
        }

        //! @brief Access to the green component.
        float& green() {
            return rgba[1];
        }

        //! @brief Const access to the green component.
        float const& green() const {
            return rgba[1];
        }

        //! @brief Access to the blue component.
        float& blue() {
            return rgba[2];
        }

        //! @brief Const access to the blue component.
        float const& blue() const {
            return rgba[2];
        }

        //! @brief Access to the alpha component.
        float& alpha() {
            return rgba[3];
        }

        //! @brief Const access to the alpha component.
        float const& alpha() const {
            return rgba[3];
        }

        //! @brief Builds a color from its HSVA representation (h maxes to 360, the rest is normalised).
        static color hsva(double h, double s, double v, double a = 1) {
            h -= 360 * floor(h / 360);
            double c = s * v;
            double x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
            double m = v - c;
            double r, g, b;
            if (h >= 0 and h < 60)
                r = c, g = x, b = 0;
            else if (h >= 60 and h < 120)
                r = x, g = c, b = 0;
            else if (h >= 120 and h < 180)
                r = 0, g = c, b = x;
            else if (h >= 180 and h < 240)
                r = 0, g = x, b = c;
            else if (h >= 240 and h < 300)
                r = x, g = 0, b = c;
            else
                r = c, g = 0, b = x;
            return { r + m, g + m, b + m, a };
        }

        //! @brief The float RGBA components of the color.
        float rgba[4];
    };

    //! @brief Builds a packed color from its RGB representation.
    constexpr packed_color packed_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        return ((uint32_t)r << 24) + ((uint32_t)g << 16) + ((uint32_t)b << 8) + a;
    }

    //! @brief Builds a packed color from its HSVA representation (h maxes to 360, the rest to 100).
    constexpr packed_color packed_hsva(int h, int s, int v, int a = 100) {
        h %= 360;
        int c = s * v;
        int x = c * (60 - abs(h % 120 - 60)) / 60;
        int m = v * 100 - c;
        int r{ 0 }, g{ 0 }, b{ 0 };
        if (h >= 0 and h < 60)
            r = c, g = x, b = 0;
        else if (h >= 60 and h < 120)
            r = x, g = c, b = 0;
        else if (h >= 120 and h < 180)
            r = 0, g = c, b = x;
        else if (h >= 180 and h < 240)
            r = 0, g = x, b = c;
        else if (h >= 240 and h < 300)
            r = x, g = 0, b = c;
        else
            r = c, g = 0, b = x;
        return packed_rgba((r + m) * 255 / 10000, (g + m) * 255 / 10000, (b + m) * 255 / 10000);
    }

	namespace internal {
        //! @brief Renderer class; it has the responsability of calling OpenGL directives.
        class Renderer {
		public:
			//! @brief Renderer constructor, with GLFW and OpenGL initializations.
			Renderer();

            //! @brief Swaps the frame buffers and prepares everything for the next frame.
            void swapAndNext();

            //! @brief It draws the orthogonal axis on the screen.
            //void drawOrtho();

            //! @brief It draws the grid on the screen.
            void drawGrid(glm::vec3 gridMin, glm::vec3 gridMax, float planeAlpha);

            //! @brief It draws a cube, given the information on color(s) and position.
            void drawCube(glm::vec3 p, double d, std::vector<color> c);

            //! @brief Returns the aspect ratio of the window.
            float getAspectRatio();

            //! @brief Returns the camera's Field of View.
            float getViewAngle();
            
            //! @brief Returns the pointer to the Renderer's m_window
            GLFWwindow* getWindow();
            
            //! @brief It runs m_camera's setViewDefault() with the given attributes.
            void setDefaultCameraView(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch);

            //! @brief Sets the light's position.
            void setLightPosition(glm::vec3& newPos);

            //! @brief Sets the scale of the grid.
            void setGridScale(double newScale);

            //! @brief Sets the far plane's distance of the projection (perspective) matrix.
            void setFarPlane(float newFar);

            //! @brief Sets the near plane's distance of the projection (perspective) matrix.
            void setNearPlane(float newNear);
            
        private:
            //! @brief Default path to vertex shader.
            static const std::string VERTEX_PATH;

            //! @brief Default path to fragment shader.
            static const std::string FRAGMENT_PATH;

            //! @brief Default path to vertex_col shader.
            static const std::string VERTEX_COLOR_PATH;

            //! @brief Default path to fragment_col shader.
            static const std::string FRAGMENT_COLOR_PATH;

            //! @brief Default path to vertex_ortho shader.
            static const std::string VERTEX_ORTHO_PATH;

            //! @brief Default path to fragment_ortho shader.
            static const std::string FRAGMENT_ORTHO_PATH;

            //! @brief Default width of the window.
            static const unsigned int SCR_DEFAULT_WIDTH{ 800 };

            //! @brief Default height of the window.
            static const unsigned int SCR_DEFAULT_HEIGHT{ 600 };

            //! @brief Default size of orthogonal axis.
            static const unsigned int SCR_DEFAULT_ORTHO{ 32 };

            //! @brief Number of vertex buffers.
            //static const unsigned int BUFF_VERTEX_NUM{ ??? };

            //! @brief Number of index buffers.
            //static const unsigned int BUFF_INDEX_NUM{ ??? };

            //! @brief Default light position.
            static const glm::vec3 LIGHT_DEFAULT_POS;

            //! @brief Window object for GLFW; it stores OpenGL context information.
            GLFWwindow* m_window;

            //! @brief Main shader program, with lighting caluclations and color info.
            Shader m_shaderProgram;

            //! @brief Additional shader program used for simple shapes and uniform color value.
            Shader m_shaderProgramCol;

            //! @brief Additional shader program used for orthogonal axis.
            Shader m_shaderProgramOrtho;

            //! @brief Vertex Array Object(s).
            unsigned int VAO[5];

            //! @brief Vertex Buffer Object(s).
            unsigned int VBO[5];

            //! @brief Element Buffer Object(s).
            unsigned int EBO[5];

            //! @brief Current width of the window.
            unsigned int m_currentWidth;

            //! @brief Current height of the window.
            unsigned int m_currentHeight;

            //! @brief Current size of orthogonal axis.
            unsigned int m_orthoSize;

            //! @brief The pass from a line to another of the grid.
            double m_gridScale;

            //! @brief It checks if it's the first time it drawn the grid.
            double m_gridFirst;

            //! @brief Size (in bytes) of the index data of grid's plane; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            int m_planeIndexSize;

            //! @brief Size (in bytes) of the index data of grid's non-highlighted lines; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            int m_gridNormIndexSize;

            //! @brief Size (in bytes) of the index data of grid's highlighted lines; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            int m_gridHighIndexSize;

            //! @brief Current position of light source.
            glm::vec3 m_lightPos;

            //! @brief Camera object of the scene
            Camera m_camera;

            //! @brief Last mouse X position.
            float m_mouseLastX;

            //! @brief Last mouse Y position.
            float m_mouseLastY;

            //! @brief It checks if it's the first mouse's input capture.
            bool m_mouseFirst;

            //! @brief Time between current frame and last frame.
            float m_deltaTime;

            //! @brief Time of last frame.
            float m_lastFrame;

            //! @brief The far plane's distance of the projection matrix.
            float m_zFar;

            //! @brief The near plane's distance of the projection matrix.
            float m_zNear;

            //! @brief Keyboard input updater function (not a proper callback: it has to be called explicitly).
            void processKeyboardInput();

            //! @brief Mouse input callback function.
            void mousePosCallback(GLFWwindow* window, double xpos, double ypos);

            //! @brief Mouse scroll callback function.
            void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

            //! @brief Window resize callback function.
            void framebufferSizeCallback(GLFWwindow* window, int width, int height);

            //! @brief Bind internally-defined callback functions to OpenGL events.
            void setInternalCallbacks();
		};
	}
}

#endif // FCPP_GRAPHICS_RENDERER_H_
