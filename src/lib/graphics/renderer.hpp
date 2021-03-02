// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_RENDERER_H_
#define FCPP_GRAPHICS_RENDERER_H_

#include <mutex>
#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "lib/data/color.hpp"
#include "lib/graphics/camera.hpp"
#include "lib/graphics/shader.hpp"
#include "lib/graphics/shapes.hpp"
#include "lib/graphics/input_types.hpp"


namespace fcpp {
	namespace internal {
        struct glyph {
            unsigned int textureID;  // ID handle of the glyph texture
            glm::ivec2   size;       // size of glyph
            glm::ivec2   bearing;    // offset from baseline to left/top of glyph
            unsigned int advance;    // offset to advance to next glyph
        };

        //! @brief Renderer class; it has the responsability of calling OpenGL directives.
        class Renderer {
		public:
			//! @brief Renderer constructor, with GLFW and OpenGL initializations.
			Renderer(size_t antialias, std::string name);

            //! @brief Swaps the frame buffers and prepares everything for the next frame.
            void swapAndNext();

            //! @brief It creates the grid mesh to be drawn.
            void makeGrid(glm::vec3 gridMin, glm::vec3 gridMax, double gridScale, std::string texture = "");

            //! @brief It draws the grid on the screen.
            void drawGrid(float planeAlpha);

            //! @brief It draws the defined shape, given the information on color(s), dimension and position.
            void drawShape(shape sh, glm::vec3 const& p, double d, std::vector<color> const& c) const;

            //! @brief It draws a star of lines, given the center and sides.
            void drawStar(glm::vec3 const& p, std::vector<glm::vec3> const& np) const;
            
            //! @brief It draws the specified text in the specified coordinates, scale and color.
            void drawText(std::string text, float x, float y, float scale);
            
            //! @brief Returns the aspect ratio of the window.
            float getAspectRatio();
            
            //! @brief Returns viewport's current width.
            int getCurrentWidth();
            
            //! @brief Returns viewport's current height.
            int getCurrentHeight();

            //! @brief Returns the pointer to the Renderer's m_window
            GLFWwindow* getWindow();
            
            //! @brief It runs m_camera's setViewDefault() with the given attributes.
            void setDefaultCameraView(glm::vec3 position, float depth, glm::vec3 worldUp, float yaw, float pitch);

            //! @brief It loads and sets the new texture to be displayed on the grid's plane.
            bool setGridTexture(std::string path);

            //! @brief Sets the light's position.
            void setLightPosition(glm::vec3& newPos);
            
            //! @brief It manages mouse input of the given type.
            void mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods);
            
            //! @brief Given the key stroke, the press status and a deltaTime, it manages keyboard input for the renderer and other classes.
            void keyboardInput(int key, bool first, float deltaTime, int mods);
            
            //! @brief It resizes the viewport, given the new width and height values.
            void viewportResize(int width, int height);
            
        private:
            //! @brief Default path to vertex_phong shader.
            static const std::string VERTEX_PHONG_PATH;

            //! @brief Default path to fragment_phong shader.
            static const std::string FRAGMENT_PHONG_PATH;

            //! @brief Default path to vertex_col shader.
            static const std::string VERTEX_COLOR_PATH;

            //! @brief Default path to fragment_col shader.
            static const std::string FRAGMENT_COLOR_PATH;

            //! @brief Default path to vertex_texture shader.
            static const std::string VERTEX_TEXTURE_PATH;

            //! @brief Default path to fragment_texture shader.
            static const std::string FRAGMENT_TEXTURE_PATH;
            
            //! @brief Default path to vertex_font shader.
            static const std::string VERTEX_FONT_PATH;

            //! @brief Default path to fragment_font shader.
            static const std::string FRAGMENT_FONT_PATH;
            
            //! @brief Default path to font.
            static const std::string FONT_PATH;

            //! @brief Default path to textures.
            static const std::string TEXTURE_PATH;
            
            //! @brief Default font size.
            static constexpr unsigned int FONT_DEFAULT_SIZE{ 48 };

            //! @brief Default width of the window.
            static constexpr unsigned int SCR_DEFAULT_WIDTH{ 800 };

            //! @brief Default height of the window.
            static constexpr unsigned int SCR_DEFAULT_HEIGHT{ 600 };

            //! @brief Default light position.
            static const glm::vec3 LIGHT_DEFAULT_POS;

            //! @brief Default light color.
            static const glm::vec3 LIGHT_COLOR;

            //! @brief Window object for GLFW; it stores OpenGL context information.
            GLFWwindow* m_window;

            //! @brief Main shader program, with phong lighting caluclations and color info.
            Shader m_shaderProgramPhong;

            //! @brief Additional shader program used for simple shapes and uniform color value.
            Shader m_shaderProgramCol;

            //! @brief Additional shader program used for texture rendering.
            Shader m_shaderProgramTexture;

            //! @brief Additional shader program used for fonts.
            Shader m_shaderProgramFont;

            //! @brief It contains all the vertex information of the standard shapes.
            Shapes m_shapes;

            //! @brief Vertex Array Objects for standard shapes.
            unsigned int m_shapeVAO[(int)shape::SIZE];

            //! @brief Vertex Buffer Objects for standard shapes.
            unsigned int m_shapeVBO[(int)shape::SIZE];

            //! @brief Vertex Array Objects for commonly used meshes.
            unsigned int m_meshVAO[(int)vertex::SIZE];

            //! @brief Vertex Buffer Objects for commonly used meshes.
            unsigned int m_meshVBO[(int)vertex::SIZE];

            //! @brief Element Buffer Objects for commonly used meshes.
            unsigned int m_meshEBO[(int)index::SIZE];
            
            //! @brief Data structure mapping chars with glyphs.
            std::unordered_map<char, glyph> m_glyphs;

            //! @brief Current width of the window.
            unsigned int m_currentWidth;

            //! @brief Current height of the window.
            unsigned int m_currentHeight;

            //! @brief Texture ID of the (optional) texture map for the grid.
            unsigned int m_gridTexture;

            //! @brief It checks if it's the first time it drawn the grid.
            bool m_gridFirst;

            //! @brief It states if the grid should be drawn.
            bool m_gridShow;

            //! @brief Size (in bytes) of the index data of grid's plane; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            int m_planeIndexSize;

            //! @brief Size (in bytes) of the index data of grid's non-highlighted lines; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            int m_gridNormIndexSize;

            //! @brief Size (in bytes) of the index data of grid's highlighted lines; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            int m_gridHighIndexSize;

            //! @brief Current position of light source.
            glm::vec3 m_lightPos;

            //! @brief The background color.
            glm::vec4 m_background;

            //! @brief The foreground color.
            glm::vec4 m_foreground;

            //! @brief Camera object of the scene
            Camera m_camera;
            
            //! @brief Euclid's algorithm to get the greatest common divisor.
            int euclid(int a, int b);

            //! @brief It loads the defined texture and returns its ID, or 0 if not loaded.
            unsigned int loadTexture(std::string path);

            //! @brief It unloads the defined texture, given its path.
            bool unloadTexture(unsigned int id);

            //! @brief It loads the vertex and index data for the commonly used meshes into their respective buffers (the grid is generated separately with makeGrid()).
            void allocateMeshBuffers();

            //! @brief It loads the vertex and index data for the standard shapes into their respective buffers.
            void allocateShapeBuffers();
		};
	}
}

#endif // FCPP_GRAPHICS_RENDERER_H_
