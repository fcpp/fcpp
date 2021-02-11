// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <cmath>
#include <stdexcept>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "lib/graphics/camera.hpp"
#include "lib/graphics/renderer.hpp"
#include "lib/graphics/shader.hpp"
#include "lib/graphics/shapes.hpp"
#include "lib/graphics/input_types.hpp"


// using namespace fcpp::internal to prevent very verbose code...
using namespace fcpp::internal;


/* --- PRIVATE (NON-INTEGRAL) CONSTANTS --- */
#ifdef _WIN32
    const std::string Renderer::VERTEX_PHONG_PATH{ ".\\shaders\\vertex_phong.glsl" };
    const std::string Renderer::FRAGMENT_PHONG_PATH{ ".\\shaders\\fragment_phong.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ ".\\shaders\\vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ ".\\shaders\\fragment_col.glsl" };
    const std::string Renderer::VERTEX_TEXTURE_PATH{ ".\\shaders\\vertex_texture.glsl" };
    const std::string Renderer::FRAGMENT_TEXTURE_PATH{ ".\\shaders\\fragment_texture.glsl" };
    const std::string Renderer::VERTEX_FONT_PATH{ ".\\shaders\\vertex_font.glsl" };
    const std::string Renderer::FRAGMENT_FONT_PATH{ ".\\shaders\\fragment_font.glsl" };
    const std::string Renderer::FONT_PATH{ ".\\fonts\\hack\\Hack-Regular.ttf" };
    const std::string Renderer::TEXTURE_PATH{ ".\\textures\\" };
#else
    const std::string Renderer::VERTEX_PHONG_PATH{ "./shaders/vertex_phong.glsl" };
    const std::string Renderer::FRAGMENT_PHONG_PATH{ "./shaders/fragment_phong.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" };
    const std::string Renderer::VERTEX_TEXTURE_PATH{ "./shaders/vertex_texture.glsl" };
    const std::string Renderer::FRAGMENT_TEXTURE_PATH{ "./shaders/fragment_texture.glsl" };
    const std::string Renderer::VERTEX_FONT_PATH{ "./shaders/vertex_font.glsl" };
    const std::string Renderer::FRAGMENT_FONT_PATH{ "./shaders/fragment_font.glsl" };
    const std::string Renderer::FONT_PATH{ "./fonts/hack/Hack-Regular.ttf" };
    const std::string Renderer::TEXTURE_PATH{ "./textures/" };
#endif
    const glm::vec3 Renderer::LIGHT_DEFAULT_POS{ 0.0f, 0.0f, 0.0f };
    const glm::vec3 Renderer::LIGHT_COLOR{ 1.0f, 1.0f, 1.0f };


/* --- CONSTRUCTOR --- */
Renderer::Renderer(size_t antialias) :
    m_currentWidth{ SCR_DEFAULT_WIDTH },
    m_currentHeight{ SCR_DEFAULT_HEIGHT },
    m_gridFirst{ true },
    m_gridShow{ true },
    m_gridTexture{ 0 },
    m_planeIndexSize{ 0 },
    m_gridHighIndexSize{ 0 },
    m_gridNormIndexSize{ 0 },
    m_lightPos{ LIGHT_DEFAULT_POS },
    m_background{ 1.0f, 1.0f, 1.0f, 1.0f },
    m_foreground{ 0.0f, 0.0f, 0.0f, 1.0f },
    m_camera{}
{
    /* DEFINITION */
    // Initialize GLFW
    glfwInit();

    // Set context options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    if (antialias > 1)
        glfwWindowHint(GLFW_SAMPLES, antialias);

    // Create window (and its context)
    m_window = glfwCreateWindow(SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT, "fcppGL", NULL, NULL);

    if (m_window == NULL) {
        glfwTerminate();
        throw std::runtime_error("ERROR::RENDERER::GLFW::WINDOW_CREATION_FAILED\n");
    }

    // Set newly created window's context as current
    glfwMakeContextCurrent(m_window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("ERROR::RENDERER::GLAD::INIT_FAILED\n");

    // Initialize FreeType
    FT_Library ftLib;
    if (FT_Init_FreeType(&ftLib))
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::LIB_INIT_FAILED\n");
    FT_Face ftFace;
    if (FT_New_Face(ftLib, FONT_PATH.c_str(), 0, &ftFace))
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::FONT_LOAD_FAILED (" + FONT_PATH + ")\n");

    // Generating glyphs' textures
    FT_Set_Pixel_Sizes(ftFace, 0, FONT_DEFAULT_SIZE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph 
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::RENDERER::FREETYPE::GLYPH_LOAD_FAILED (" << c << ")\n";
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ftFace->glyph->bitmap.width,
            ftFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ftFace->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        glyph gl = {
            texture, 
            glm::ivec2(ftFace->glyph->bitmap.width, ftFace->glyph->bitmap.rows),
            glm::ivec2(ftFace->glyph->bitmap_left, ftFace->glyph->bitmap_top),
            (unsigned int)(ftFace->glyph->advance.x)
        };
        m_glyphs.insert(std::pair<char, glyph>(c, gl));
    }
    
    // Deallocate FreeType structures
    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLib);

    // Set viewport
    glViewport(0, 0, SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT);

    // Generate actual shader programs
    m_shaderProgramPhong = Shader{ VERTEX_PHONG_PATH.c_str(), FRAGMENT_PHONG_PATH.c_str() };
    m_shaderProgramCol = Shader{ VERTEX_COLOR_PATH.c_str(), FRAGMENT_COLOR_PATH.c_str() };
    m_shaderProgramTexture = Shader{ VERTEX_TEXTURE_PATH.c_str(), FRAGMENT_TEXTURE_PATH.c_str() };
    m_shaderProgramFont = Shader{ VERTEX_FONT_PATH.c_str(), FRAGMENT_FONT_PATH.c_str() };

    // Generate VAOs, VBOs and EBOs
    glGenVertexArrays((int)vertex::SIZE, VAO);
    glGenBuffers((int)vertex::SIZE, VBO);
    glGenBuffers((int)index::SIZE, EBO);

    // Allocate (static) cube buffers
    glBindVertexArray(VAO[(int)vertex::cube]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::cube]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::VERTEX_CUBE), Shapes::VERTEX_CUBE, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Allocate (dynamic) font buffers
    glBindVertexArray(VAO[(int)vertex::font]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::font]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Allocate (dynamic) single line buffers
    glBindVertexArray(VAO[(int)vertex::singleLine]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::singleLine]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allocate (dynamic) neighbour star buffers
    glBindVertexArray(VAO[(int)vertex::star]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::star]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enabling depth test
    glEnable(GL_DEPTH_TEST);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable antialiasing
    if (antialias > 1)
        glEnable(GL_MULTISAMPLE);

    // Uncomment this call to draw in wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Clear first frame
    glClearColor(m_background[0], m_background[1], m_background[2], m_background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set initial aspect ratio
	m_camera.setScreen(m_currentWidth, m_currentHeight);
}


/* --- PRIVATE FUNCTIONS --- */
int Renderer::euclid(int a, int b) {
    std::cout << "euclid(" << a << ", " << b << ")";
    int r;
    while(b != 0) //repeat until b is 0
    {
         r = a % b;
         a = b; 
         b = r; //swap a and b
    }
    
    std::cout << a << "\n";
    return a; //the result is a when b is equal to 0
}

unsigned int Renderer::loadTexture(std::string path) {
    unsigned int texture{ 0 };
    stbi_set_flip_vertically_on_load(true);
    // load texture data
    int width, height, nrChannels;
    unsigned char* data{ stbi_load((TEXTURE_PATH + path).c_str(), &width, &height, &nrChannels, 0) };
    if (data) {
        // generate texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLint inputChannels;
        if (nrChannels == 3) inputChannels = GL_RGB;
        else inputChannels = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, inputChannels, width, height, 0, inputChannels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else std::cerr << "ERROR::RENDERER::STBIMAGE::TEXTURE_LOAD_FAILED (" << path << ")\n";
    stbi_image_free(data);

    return texture;
}

bool Renderer::unloadTexture(unsigned int id) {
    bool success{ glIsTexture(id) };
    glDeleteTextures(1, &id); // be aware: glDeleteTextures() silently ignores 0's and names that do not correspond to existing textures
    if(!success) std::cerr << "ERROR::RENDERER::TEXTURE::TEXTURE_NOT_FOUND (id = " << id << ")\n";

    return success;
}


/* --- PUBLIC FUNCTIONS --- */
void Renderer::swapAndNext() {
    // Check and call events, swap double buffers
    glfwPollEvents();
    glfwSwapBuffers(m_window);

    // Clear frame
    glClearColor(m_background[0], m_background[1], m_background[2], m_background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::makeGrid(glm::vec3 gridMin, glm::vec3 gridMax, double gridScale, std::string texture) {
    // Initialize grid and plane buffers if they are not already
    if (m_gridFirst) {
        // The grid is initiated
        m_gridFirst = false;

        // Calculate data for mesh generation
        int approx{ (gridMax.x - gridMin.x) * (gridMax.y - gridMin.y) > 2000 * gridScale * gridScale ? 10 : 1 };
        int grid_min_x = std::floor(gridMin.x / gridScale / approx) * approx;
        int grid_max_x = std::ceil(gridMax.x / gridScale / approx) * approx;
        int grid_min_y = std::floor(gridMin.y / gridScale / approx) * approx;
        int grid_max_y = std::ceil(gridMax.y / gridScale / approx) * approx;
        int numX{ grid_max_x - grid_min_x + 1 };
        int numY{ grid_max_y - grid_min_y + 1 };

        int highlighter{ 10 }; // the module required by a line to be highlighted
        /* symmetrical code:
        if (numX != numY or numX % 10 != 0) {
            highlighter = euclid(numX - 1, numY - 1);
            while (highlighter >= 10 and highlighter % 2 == 0)
                highlighter /= 2;
        } else highlighter = 10;
        */

        int numHighX{ (grid_max_x - grid_max_x % highlighter - grid_min_x) / highlighter + 1 };
        int numHighY{ (grid_max_y - grid_max_y % highlighter - grid_min_y) / highlighter + 1 };
        /* symmetrical code:
        if (highlighter != 1) {
            numHighX = (grid_max_x - grid_max_x % highlighter - grid_min_x) / highlighter + 1;
            numHighY = (grid_max_y - grid_max_y % highlighter - grid_min_y) / highlighter + 1;
        } else numHighX = numHighY = 2;
        */

        // Generating grid mesh
        int i{ 0 }; // for putting vertices into gridMesh
        int j{ 0 }; // for putting indices into gridHighMesh
        int k{ 0 }; // for putting indices into gridNormMesh
        float gridMesh[numX * 6 + numY * 6]; // will contain the vertex data of the grid
        int gridNormIndex[(numX - numHighX) * 2 + (numY - numHighY) * 2]; // will contain the index data of the normal lines of the grid
        int gridHighIndex[numHighX * 2 + numHighY * 2]; // will contain the index data of the highlighted lines of the grid
        for (int x = grid_min_x; x <= grid_max_x; ++x) {
            gridMesh[i * 6] = (float)(x * gridScale);
            gridMesh[1 + i * 6] = (float)(grid_min_y * gridScale);
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = (float)(x * gridScale);
            gridMesh[4 + i * 6] = (float)(grid_max_y * gridScale);
            gridMesh[5 + i * 6] = 0.0f;
            if (x % highlighter == 0) { // symmetrical code: (highlighter != 1 and x % highlighter == 0) or (x == grid_min_x or x == grid_max_x)
                gridHighIndex[j * 2] = i * 2;
                gridHighIndex[1 + j * 2] = i * 2 + 1;
                ++j;
            }
            else {
                gridNormIndex[k * 2] = i * 2;
                gridNormIndex[1 + k * 2] = i * 2 + 1;
                ++k;
            }
            ++i;
        }
        for (int y = grid_min_y; y <= grid_max_y; ++y) {
            gridMesh[i * 6] = (float)(grid_min_x * gridScale);
            gridMesh[1 + i * 6] = (float)(y * gridScale);
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = (float)(grid_max_x * gridScale);
            gridMesh[4 + i * 6] = (float)(y * gridScale);
            gridMesh[5 + i * 6] = 0.0f;
            if (y % highlighter == 0) { //symmetrical code: (highlighter != 1 and y % highlighter == 0) or (y == grid_min_y or y == grid_max_y)
                gridHighIndex[j * 2] = i * 2;
                gridHighIndex[1 + j * 2] = i * 2 + 1;
                ++j;
            }
            else {
                gridNormIndex[k * 2] = i * 2;
                gridNormIndex[1 + k * 2] = i * 2 + 1;
                ++k;
            }
            ++i;
        }

        // Storing grid mesh
        glBindVertexArray(VAO[(int)vertex::grid]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::grid]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gridMesh), gridMesh, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::gridHigh]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridHighIndex), gridHighIndex, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::gridNorm]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridNormIndex), gridNormIndex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        m_gridNormIndexSize = sizeof(gridNormIndex);
        m_gridHighIndexSize = sizeof(gridHighIndex);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // Generating plane mesh
        float planeMesh[20]{
            // vertex coords                                                        // texture coords
            (float)(grid_min_x * gridScale), (float)(grid_min_y * gridScale), 0.0f, 0.0f, 0.0f,
            (float)(grid_min_x * gridScale), (float)(grid_max_y * gridScale), 0.0f, 0.0f, 1.0f,
            (float)(grid_max_x * gridScale), (float)(grid_max_y * gridScale), 0.0f, 1.0f, 1.0f,
            (float)(grid_max_x * gridScale), (float)(grid_min_y * gridScale), 0.0f, 1.0f, 0.0f
        };
        int planeIndex[6]{
            0, 1, 2,
            2, 3, 0
        };

        // Storing plane mesh
        glBindVertexArray(VAO[(int)vertex::plane]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::plane]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeMesh), planeMesh, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::plane]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndex), planeIndex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        m_planeIndexSize = sizeof(planeIndex);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // Loading texture
        if ((texture.compare("") != 0) and setGridTexture(texture)) m_gridShow = false;
    }
}

void Renderer::drawGrid(float planeAlpha) {
    if (!m_gridFirst) {
        // Create matrices (used several times)
        glm::mat4 const& projection{ m_camera.getPerspective() };
        glm::mat4 const& view{ m_camera.getView() };
        glm::mat4 model{ 1.0f };

        if (m_gridShow) {
            // Set up shader program
            m_shaderProgramCol.use();
            m_shaderProgramCol.setMat4("u_projection", projection);
            m_shaderProgramCol.setMat4("u_view", view);
            m_shaderProgramCol.setMat4("u_model", model);

            // Draw grid
            glBindVertexArray(VAO[(int)vertex::grid]);
            m_shaderProgramCol.setVec4("u_color", m_foreground);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::gridHigh]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
            glDrawElements(GL_LINES, m_gridHighIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
            m_shaderProgramCol.setVec4("u_color", m_background);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::gridNorm]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
            glDrawElements(GL_LINES, m_gridNormIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        // Draw plane
        if (planeAlpha > 0.0f) {
            // Set up shader program
            m_shaderProgramTexture.use();
            m_shaderProgramTexture.setMat4("u_projection", projection);
            m_shaderProgramTexture.setMat4("u_view", view);
            m_shaderProgramTexture.setMat4("u_model", model);

            glm::vec4 col;
            if (m_gridTexture == 0) {
                col.x = (m_background[0] + m_foreground[0]) / 2;
                col.y = (m_background[1] + m_foreground[1]) / 2;
                col.z = (m_background[2] + m_foreground[2]) / 2;
                col.w = planeAlpha;
                m_shaderProgramTexture.setBool("u_drawTexture", false);
            } else {
                col.x = col.y = col.z = 1.0f;
                col.w = planeAlpha;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_gridTexture);
                m_shaderProgramTexture.setBool("u_drawTexture", true);
            }
            m_shaderProgramTexture.setVec4("u_color", col);
            m_shaderProgramTexture.setInt("u_texture", 0);
            glDepthMask(false);
            glBindVertexArray(VAO[(int)vertex::plane]);
            glDrawElements(GL_TRIANGLES, m_planeIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
            glDepthMask(true);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}

void Renderer::drawCube(glm::vec3 const& p, double d, std::vector<color> const& c) const {    
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getPerspective() };
    glm::mat4 const& view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };
    model = glm::translate(model, p);
    model = glm::scale(model, glm::vec3(d));
    glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(view * model)));
    glm::vec4 col{ c[0].red(), c[0].green(), c[0].blue(), c[0].alpha() }; // access to first color only is temporary...
    
    // Draw cube
    m_shaderProgramPhong.use();
    glBindVertexArray(VAO[(int)vertex::cube]);
    m_shaderProgramPhong.setVec3("u_lightPos", m_lightPos);
    m_shaderProgramPhong.setFloat("u_ambientStrength", 0.4f);
    m_shaderProgramPhong.setVec4("u_objectColor", col);
    m_shaderProgramPhong.setVec3("u_lightColor", LIGHT_COLOR);
    m_shaderProgramPhong.setMat4("u_projection", projection);
    m_shaderProgramPhong.setMat4("u_view", view);
    m_shaderProgramPhong.setMat4("u_model", model);
    m_shaderProgramPhong.setMat3("u_normal", normal);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // Draw pin
    if (p.z > 0) {
        float pinData[] = {
            p.x, p.y, p.z,
            p.x, p.y, 0.0f
        };
        m_shaderProgramCol.use();
        m_shaderProgramCol.setMat4("u_projection", projection);
        m_shaderProgramCol.setMat4("u_view", view);
        m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
        m_shaderProgramCol.setVec4("u_color", col);
        glBindVertexArray(VAO[(int)vertex::singleLine]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::singleLine]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pinData), pinData); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_LINES, 0, 6);
    }
}

//! @brief It draws a star of lines, given the center and sides.
void Renderer::drawStar(glm::vec3 const& p, std::vector<glm::vec3> const& np) const {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getPerspective() };
    glm::mat4 const& view{ m_camera.getView() };

    float starData[6 * np.size()];
    for (int i=0; i<np.size(); ++i) {
        starData[6*i+0] = p[0];
        starData[6*i+1] = p[1];
        starData[6*i+2] = p[2];
        starData[6*i+3] = np[i][0];
        starData[6*i+4] = np[i][1];
        starData[6*i+5] = np[i][2];
    }

    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);
    m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
    m_shaderProgramCol.setVec4("u_color", glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
    glBindVertexArray(VAO[(int)vertex::star]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::star]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(starData), starData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_LINES, 0, 2 * np.size());
}

void Renderer::drawText(std::string text, float x, float y, float scale)
{
    // Activate corresponding render state	
    m_shaderProgramFont.use();
    m_shaderProgramFont.setVec3("u_textColor", m_foreground);
    m_shaderProgramFont.setInt("u_text", 0);
    m_shaderProgramFont.setMat4("u_projection", m_camera.getOrthographic());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO[(int)vertex::font]);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        glyph ch = m_glyphs[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // Update VBO for each character
        float vertices[24] = {
            xpos,     ypos + h,   0.0f, 0.0f,            
            xpos,     ypos,       0.0f, 1.0f,
            xpos + w, ypos,       1.0f, 1.0f,

            xpos,     ypos + h,   0.0f, 0.0f,
            xpos + w, ypos,       1.0f, 1.0f,
            xpos + w, ypos + h,   1.0f, 0.0f           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::font]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float Renderer::getAspectRatio() {
    return (float)(m_currentWidth) / (float)(m_currentHeight);
}

int Renderer::getCurrentWidth() {
    return m_currentWidth;
}

int Renderer::getCurrentHeight() {
    return m_currentHeight;
}

GLFWwindow* Renderer::getWindow() {
    return m_window;
}

void Renderer::setDefaultCameraView(glm::vec3 position, float depth, glm::vec3 worldUp, float yaw, float pitch) {
    m_camera.setViewDefault(position, depth, worldUp, yaw, pitch);
}

bool Renderer::setGridTexture(std::string path) {
    bool success{ false };
    unsigned int loadedId{ loadTexture(path) };
    if (loadedId != 0) {
        if (m_gridTexture != 0) unloadTexture(m_gridTexture);
        m_gridTexture = loadedId;
        success = true;
    }

    return success;
}

void Renderer::setLightPosition(glm::vec3& newPos) {
    m_lightPos = newPos;
}

void Renderer::mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods) {
    m_camera.mouseInput(x, y, xFirst, yFirst, type, mods);
}

void Renderer::keyboardInput(int key, bool first, float deltaTime, int mods) {
    // Process renderer's input
    switch (key) {
        // show/hide grid
        case GLFW_KEY_G:
            if (first) m_gridShow = not m_gridShow;
            break;
    }
    
    // Process camera's input
    m_camera.keyboardInput(key, first, deltaTime, mods);
}

void Renderer::viewportResize(int width, int height) {
    glViewport(0, 0, width, height);
    m_currentWidth = width;
    m_currentHeight = height;
	m_camera.setScreen(width, height);
}
