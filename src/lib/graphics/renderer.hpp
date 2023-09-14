// Copyright © 2023 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file renderer.hpp
 * @brief Implementation of the `renderer` class.
 */

#ifndef FCPP_GRAPHICS_RENDERER_H_
#define FCPP_GRAPHICS_RENDERER_H_

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "lib/data/color.hpp"
#include "lib/graphics/camera.hpp"
#include "lib/graphics/shader.hpp"
#include "lib/graphics/shapes.hpp"


namespace fcpp {
    namespace internal {
        //! @brief Supported pointers to vertex buffers.
        enum class vertex : char { singleLine, star, plane, grid, rectangle, SIZE };

        //! @brief Supported pointers to index buffers.
        enum class index : char { plane, gridNorm, gridHigh, rectangle, SIZE };

        //! @brief Glyph struct.
        struct glyph {
            //! @brief ID handle of the glyph texture.
            unsigned int textureID;
            //! @brief Size of glyph.
            glm::ivec2   size;
            //! @brief Offset from baseline to left/top of glyph.
            glm::ivec2   bearing;
            //! @brief Offset to advance to next glyph.
            unsigned int advance;
        };

        //! @brief Renderer class; it has the responsability of calling OpenGL directives.
        class renderer {
        public:
            //! @brief Renderer constructor, with GLFW and OpenGL initializations.
            renderer(size_t antialias, std::string name, bool master = true, GLFWwindow* masterPtr = NULL);

            //! @brief Move constructor.
            renderer(renderer&& source) = delete;

            //! @brief Renderer destructor closing the window.
            ~renderer();

            //! @brief Move assignment.
            renderer& operator=(renderer&& source) = delete;

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

            //! @brief It draws the defined shape shadow, given the information on color, dimension and position.
            void drawShadow(shape sh, glm::vec3 p, double d, color const& c) const;

            //! @brief It draws a star of lines, given the center and sides.
            void drawStar(glm::vec3 const& p, std::vector<glm::vec3> const& np) const;

            //! @brief It draws the tail of a node, as a sequence of lines given their endpoints, the color to be used and a width.
            void drawTail(std::deque<glm::vec3> const& p, std::deque<vec<2>> const& n, color const& c, float w) const;

            //! @brief It draws the specified text in the specified window coordinates, scale and color.
            void drawText(std::string text, float x, float y, float scale) const;

            //! @brief It draws the specified label in the specified window coordinates, scale and color.
            void drawLabel(std::string text, glm::vec3 const& p, glm::vec4 col, float scale) const;

            //! @brief It draws the defined rectangle, given the information on color(s) and position
            void drawRectangle(float x, float y, float a, float b) const;

            //! @brief Returns the aspect ratio of the window.
            float getAspectRatio();

            //! @brief Returns window's current width.
            int getWindowWidth();

            //! @brief Returns window's current height.
            int getWindowHeight();

            //! @brief Returns framebuffer's current width.
            int getFramebufferWidth();

            //! @brief Returns framebuffer's current height.
            int getFramebufferHeight();

            //! @brief Returns the render scale.
            double getRenderScale();

            //! @brief Returns the pointer to the renderer's m_window
            GLFWwindow* getWindow() const;

            //! @brief It returns the reference of the camera object.
            camera& getCamera();

            //! @brief It returns the (constant) reference of the camera object.
            camera const& getCamera() const;

            //! @brief It loads and sets the new texture to be displayed on the grid's plane.
            bool setGridTexture(std::string path);

            //! @brief Sets the light's position.
            void setLightPosition(glm::vec3& newPos);

            //! @brief It changes the idle shape of the cursor; boolean 'selection' states whether mouse picking is active or not; boolean 'setNow' states whether the call should immediately apply the change to the actual cursor.
            void setStandardCursor(bool selection, bool setNow = true);

            //! @brief It manages mouse input of the given type.
            void mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods);

            //! @brief Given the key stroke, the press status and a deltaTime, it manages keyboard input for the renderer and other classes.
            bool keyboardInput(int key, bool first, float deltaTime, int mods);

            //! @brief It resizes the viewport, given the new width and height values.
            void viewportResize(int winWidth, int winHeight, int fbWidth, int fbHeight);

            //! @brief Sets the main color theme to be used in the user interface.
            void setColorTheme(color background, color foreground, color selection);

        private:
            //! @brief Default path to vertex_phong shader.
            static std::string const VERTEX_PHONG_PATH;

            //! @brief Default path to fragment_phong shader.
            static std::string const FRAGMENT_PHONG_PATH;

            //! @brief Default path to vertex_col shader.
            static std::string const VERTEX_COLOR_PATH;

            //! @brief Default path to fragment_col shader.
            static std::string const FRAGMENT_COLOR_PATH;

            //! @brief Default path to vertex_texture shader.
            static std::string const VERTEX_TEXTURE_PATH;

            //! @brief Default path to fragment_texture shader.
            static std::string const FRAGMENT_TEXTURE_PATH;

            //! @brief Default path to vertex_font shader.
            static std::string const VERTEX_FONT_PATH;

            //! @brief Default path to fragment_font shader.
            static std::string const FRAGMENT_FONT_PATH;

            //! @brief Default path to vertex_label shader.
            static std::string const VERTEX_LABEL_PATH;

            //! @brief Default path to fragment_label shader.
            static std::string const FRAGMENT_LABEL_PATH;

            //! @brief Default path to font.
            static std::string const FONT_PATH;

            //! @brief Default path to textures.
            static std::string const TEXTURE_PATH;

            //! @brief Default font size.
            static constexpr unsigned int FONT_DEFAULT_SIZE{ 48 };

            //! @brief Default width of the window.
            static constexpr unsigned int SCR_DEFAULT_WIDTH{ 800 };

            //! @brief Default height of the window.
            static constexpr unsigned int SCR_DEFAULT_HEIGHT{ 600 };

            //! @brief Default light position.
            static glm::vec3 const LIGHT_DEFAULT_POS;

            //! @brief Default light color.
            static glm::vec3 const LIGHT_COLOR;

            //! @brief It contains all the vertex information of the standard shapes.
            static shapes s_shapes;

            //! @brief Vertex Buffer Objects for standard shapes; it can be shared among several contexts.
            static unsigned int s_shapeVBO[(int)shape::SIZE];

            //! @brief Vertex Buffer Objects for standard shadows; it can be shared among several contexts.
            static unsigned int s_shadowVBO[(int)shape::SIZE];

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

            //! @brief The pointer to the arrow-shaped cursor object.
            static GLFWcursor* s_cursorArrow;

            //! @brief The pointer to the cross-shaped cursor object.
            static GLFWcursor* s_cursorCross;

            //! @brief The pointer to the hand-shaped cursor object.
            static GLFWcursor* s_cursorHand;

            //! @brief It initializes all the static structures and variables, common to the renderer instances.
            static void initializeCommon();

            //! @brief It loads the defined texture and returns its ID, or 0 if not loaded.
            static unsigned int loadTexture(std::string path);

            //! @brief It unloads the defined texture, given its path.
            static bool unloadTexture(unsigned int id);

            //! @brief It loads glyphs' texture buffers.
            static void allocateGlyphTextures();

            //! @brief It loads the VBOs and EBOs for the standard shapes into their respective buffers.
            static void allocateShapeVertex();

            //! @brief It loads the VBOs and EBOs for the commonly used meshes into their respective buffers (the grid is generated separately with makeGrid()).
            static void allocateMeshVertex();

            //! @brief Window object for GLFW; it stores OpenGL context information.
            GLFWwindow* m_window;

            //! @brief It specifies if the renderer is the main one (master) or not.
            bool m_master;

            //! @brief It states if the next swapAndNext() call should perform glViewport() with the new screen parameters; it's used by slave renderers only.
            bool m_resizeOnSwap;

            //! @brief Vertex Array Objects for standard shapes; it's per context and it can't be shared with others.
            unsigned int m_shapeVAO[(int)shape::SIZE];

            //! @brief Vertex Array Objects for standard shadows; it's per context and it can't be shared with others.
            unsigned int m_shadowVAO[(int)shape::SIZE];

            //! @brief Vertex Array Objects for commonly used meshes; it's per context and it can't be shared with others.
            unsigned int m_meshVAO[(int)vertex::SIZE];

            //! @brief Vertex Array Object dedicated to font rendering; it's per context and it can't be shared with others.
            unsigned int m_fontVAO;

            //! @brief Vertex Buffer Object dedicated to font rendering; it's per renderer and it shouldn't be shared with others.
            unsigned int m_fontVBO;

            //! @brief Vertex Array Object dedicated to label rendering; it's per context and it can't be shared with others.
            unsigned int m_labelVAO;

            //! @brief Vertex Buffer Object dedicated to label rendering; it's per renderer and it shouldn't be shared with others.
            unsigned int m_labelVBO;

            //! @brief Main shader program, with diffuse lighting caluclations and color info; it's per context and it shouldn't be shared with others.
            shader m_shaderProgramDiff;

            //! @brief Additional shader program used for simple shapes and uniform color value; it's per context and it shouldn't be shared with others.
            shader m_shaderProgramCol;

            //! @brief Additional shader program used for texture rendering; it's per context and it shouldn't be shared with others.
            shader m_shaderProgramTexture;

            //! @brief Additional shader program used for fonts; it's per context and it shouldn't be shared with others.
            shader m_shaderProgramFont;

            //! @brief Additional shader program used for labels; it's per context and it shouldn't be shared with others.
            shader m_shaderProgramLabel;

            //! @brief Current width of the window.
            int m_windowWidth;

            //! @brief Current height of the window.
            int m_windowHeight;

            //! @brief Current width of the default framebuffer.
            int m_framebufferWidth;

            //! @brief Current height of the default framebuffer.
            int m_framebufferHeight;

            //! @brief Scaling factor of the framebuffer in relation to the window.
            double m_renderScale;

            //! @brief Texture ID of the (optional) texture map for the grid.
            unsigned int m_gridTexture;

            //! @brief It states if the grid should be drawn.
            char m_gridShow;

            //! @brief It's the cursor's standard shape, in idle.
            GLFWcursor* m_standardCursor;

            //! @brief Current position of light source.
            glm::vec3 m_lightPos;

            //! @brief The background color.
            glm::vec4 m_background;

            //! @brief The rectangle color.
            glm::vec4 m_rectangle_col;

            //! @brief The foreground color.
            glm::vec4 m_foreground;

            //! @brief Camera object of the scene
            camera m_camera;

            //! @brief Euclid's algorithm to get the greatest common divisor.
            int euclid(int a, int b);

            //! @brief It loads the VAOs for the standard shapes into their respective buffers.
            void generateShapeAttributePointers();

            //! @brief It loads the VAOs for the commonly used meshes into their respective buffers (the grid is generated separately with makeGrid()).
            void generateMeshAttributePointers();

            //! @brief It allocates the VAO and VBO dedicated to the font rendering.
            void generateFontBuffers();

            //! @brief It compiles the shader programs.
            void generateShaderPrograms();
        };
    }
}

#endif // FCPP_GRAPHICS_RENDERER_H_
