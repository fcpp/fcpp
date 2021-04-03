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
        //! @brief Supported pointers to vertex buffers.
        enum class vertex { singleLine, star, plane, grid, SIZE };

        //! @brief Supported pointers to index buffers.
        enum class index { plane, gridNorm, gridHigh, SIZE };

        //! @brief Glyph struct.
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
			Renderer(size_t antialias, std::string name, bool master = true, GLFWwindow* masterPtr = NULL);

            //! @brief Renderer destructor closing the window.
            ~Renderer();

            //! @brief It initializes the context on the current thread.
            void initializeContext(bool master = true);

            //! @brief Swaps the frame buffers and prepares everything for the next frame.
            void swapAndNext();

            //! @brief It creates the grid mesh to be drawn.
            void makeGrid(glm::vec3 gridMin, glm::vec3 gridMax, double gridScale);

            //! @brief It draws the grid on the screen.
            void drawGrid(float planeAlpha) const;

            //! @brief It draws the defined shape, given the information on color(s), dimension and position.
            void drawShape(shape sh, glm::vec3 const& p, double d, std::vector<color> const& c) const;

            //! @brief It draws a star of lines, given the center and sides.
            void drawStar(glm::vec3 const& p, std::vector<glm::vec3> const& np) const;
            
            //! @brief It draws the specified text in the specified coordinates, scale and color.
            void drawText(std::string text, float x, float y, float scale) const;
            
            //! @brief Returns the aspect ratio of the window.
            float getAspectRatio();
            
            //! @brief Returns viewport's current width.
            int getCurrentWidth();
            
            //! @brief Returns viewport's current height.
            int getCurrentHeight();

            //! @brief Returns the pointer to the Renderer's m_window
            GLFWwindow* getWindow() const;
            
            //! @brief It returns the reference of the camera object.
            Camera& getCamera();

            //! @brief It returns the (constant) reference of the camera object.
            Camera const& getCamera() const;

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

            //! @brief It contains all the vertex information of the standard shapes.
            static Shapes s_shapes;

            //! @brief Vertex Buffer Objects for standard shapes; it can be shared among several contexts.
            static unsigned int s_shapeVBO[(int)shape::SIZE];

            //! @brief Vertex Buffer Objects for commonly used meshes; it can be shared among several contexts.
            static unsigned int s_meshVBO[(int)vertex::SIZE];

            //! @brief Element Buffer Objects for commonly used meshes; it can be shared among several contexts.
            static unsigned int s_meshEBO[(int)index::SIZE];

            //! @brief Data structure mapping chars with glyphs.
            static std::unordered_map<char, glyph> s_glyphs;
            
            //! @brief Size (in bytes) of the index data of grid's plane; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            static int s_planeIndexSize;

            //! @brief Size (in bytes) of the index data of grid's non-highlighted lines; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            static int s_gridNormIndexSize;

            //! @brief Size (in bytes) of the index data of grid's highlighted lines; it is used since the size of such buffer is not defined until the first frame is up to be rendered.
            static int s_gridHighIndexSize;

            //! @brief It states whether the static structures and variables have been initialized.
            static bool s_commonIsReady;

            //! @brief It states whether the grid mesh has been calculated.
            static bool s_gridIsReady;

            //! @brief It initializes all the static structures and variables, common to the Renderer instances.
            static void initializeCommon();

            //! @brief It loads the defined texture and returns its ID, or 0 if not loaded.
            static unsigned int loadTexture(std::string path);

            //! @brief It unloads the defined texture, given its path.
            static bool unloadTexture(unsigned int id);

            //! @brief Window object for GLFW; it stores OpenGL context information.
            GLFWwindow* m_window;

            //! @brief It specifies if the renderer is the main one (master) or not.
            bool m_master;

            //! @brief It states if the next swapAndNext() call should perform glViewport() with the new screen parameters; it's used by slave renderers only.
            bool m_resizeOnSwap;

            //! @brief Vertex Array Objects for standard shapes; it's per context and it can't be shared with others.
            unsigned int m_shapeVAO[(int)shape::SIZE];

            //! @brief Vertex Array Objects for commonly used meshes; it's per context and it can't be shared with others.
            unsigned int m_meshVAO[(int)vertex::SIZE];

            //! @brief Vertex Array Object dedicated to font rendering; it's per context and it can't be shared with others.
            unsigned int m_fontVAO;

            //! @brief Vertex Buffer Object dedicated to font rendering; it's per Renderer and it shouldn't be shared with others.
            unsigned int m_fontVBO;

            //! @brief Renderer's Frame Buffer Object. All rendering calls will output on it.
            unsigned int m_FBO;

            //! @brief Render Buffer Objects attached to Renderer's FBO; the first is the color buffer, the second is the depth and stencil buffer.
            unsigned int m_RBO[2];

            //! @brief Main shader program, with diffuse lighting caluclations and color info; it's per context and it shouldn't be shared with others.
            Shader m_shaderProgramDiff;

            //! @brief Additional shader program used for simple shapes and uniform color value; it's per context and it shouldn't be shared with others.
            Shader m_shaderProgramCol;

            //! @brief Additional shader program used for texture rendering; it's per context and it shouldn't be shared with others.
            Shader m_shaderProgramTexture;

            //! @brief Additional shader program used for fonts; it's per context and it shouldn't be shared with others.
            Shader m_shaderProgramFont;

            //! @brief Current width of the window.
            unsigned int m_currentWidth;

            //! @brief Current height of the window.
            unsigned int m_currentHeight;

            //! @brief Texture ID of the (optional) texture map for the grid.
            unsigned int m_gridTexture;

            //! @brief It states if the grid should be drawn.
            bool m_gridShow;

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

            //! @brief It loads the vertex and index data for the standard shapes into their respective buffers.
            void allocateShapeBuffers(bool loadVertex = true);

            //! @brief It loads the vertex and index data for the commonly used meshes into their respective buffers (the grid is generated separately with makeGrid()).
            void allocateMeshBuffers(bool loadVertex = true);

            //! @brief It allocates the buffer dedicated to the font rendering.
            void allocateFontBuffer();

            //! @brief It allocates the rendering framebuffer.
            void allocateFrameBuffer();
		};
	}
}

#endif // FCPP_GRAPHICS_RENDERER_H_
