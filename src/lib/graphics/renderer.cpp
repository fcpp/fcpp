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
    const std::string Renderer::VERTEX_PATH{ ".\\shaders\\vertex.glsl" };
    const std::string Renderer::FRAGMENT_PATH{ ".\\shaders\\fragment.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ ".\\shaders\\vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ ".\\shaders\\fragment_col.glsl" };
    const std::string Renderer::VERTEX_ORTHO_PATH{ ".\\shaders\\vertex_ortho.glsl" };
    const std::string Renderer::FRAGMENT_ORTHO_PATH{ ".\\shaders\\fragment_ortho.glsl" };
    const std::string Renderer::VERTEX_FONT_PATH{ ".\\shaders\\vertex_font.glsl" };
    const std::string Renderer::FRAGMENT_FONT_PATH{ ".\\shaders\\fragment_font.glsl" };
    const std::string Renderer::FONT_PATH{ ".\\fonts\\hack\\Hack-Regular.ttf" };
    const std::string Renderer::TEXTURE_PATH{ ".\\textures\\" };
#else
    const std::string Renderer::VERTEX_PATH{ "./shaders/vertex.glsl" };
    const std::string Renderer::FRAGMENT_PATH{ "./shaders/fragment.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" };
    const std::string Renderer::VERTEX_ORTHO_PATH{ "./shaders/vertex_ortho.glsl" };
    const std::string Renderer::FRAGMENT_ORTHO_PATH{ "./shaders/fragment_ortho.glsl" };
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
    m_orthoSize{ SCR_DEFAULT_ORTHO },
    m_gridScale{ 1.0 },
    m_gridFirst{ true },
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
        throw std::runtime_error("Failed to create GLFW window.\n");
    }

    // Set newly created window's context as current
    glfwMakeContextCurrent(m_window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD.\n");

    // Initialize FreeType
    FT_Library ftLib;
    if (FT_Init_FreeType(&ftLib))
        throw std::runtime_error("Could not init FreeType Library.\n");
    FT_Face ftFace;
    if (FT_New_Face(ftLib, FONT_PATH.c_str(), 0, &ftFace))
        throw std::runtime_error("Failed to load font (" + FONT_PATH + ").\n");

    // Generating glyphs' textures
    FT_Set_Pixel_Sizes(ftFace, 0, FONT_DEFAULT_SIZE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    for (unsigned char c = 0; c < 128; c++) {
        // load character glyph 
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            std::cout << "Failed to load glyph (" << c << ")\n";
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
    m_shaderProgram = Shader{ VERTEX_PATH.c_str(), FRAGMENT_PATH.c_str() };
    m_shaderProgramCol = Shader{ VERTEX_COLOR_PATH.c_str(), FRAGMENT_COLOR_PATH.c_str() };
    m_shaderProgramOrtho = Shader{ VERTEX_ORTHO_PATH.c_str(), FRAGMENT_ORTHO_PATH.c_str() };
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


/* --- PUBLIC FUNCTIONS --- */
void Renderer::swapAndNext() {
    // Check and call events, swap double buffers
    glfwPollEvents();
    glfwSwapBuffers(m_window);

    // Clear frame
    glClearColor(m_background[0], m_background[1], m_background[2], m_background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawGrid(glm::vec3 gridMin, glm::vec3 gridMax, float planeAlpha) {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getProjection() };
    glm::mat4 const& view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };

    // Set up shader program
    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);
    m_shaderProgramCol.setMat4("u_model", model);

    // Initialize grid and plane buffers if they are not already
    if (m_gridFirst) {
        m_gridFirst = false;
        int approx{ (gridMax.x-gridMin.x)*(gridMax.y-gridMin.y) > 2000*m_gridScale*m_gridScale ? 10 : 1 };
        int grid_min_x = std::floor(gridMin.x / m_gridScale / approx) * approx;
        int grid_max_x = std::ceil(gridMax.x / m_gridScale / approx) * approx;
        int grid_min_y = std::floor(gridMin.y / m_gridScale / approx) * approx;
        int grid_max_y = std::ceil(gridMax.y / m_gridScale / approx) * approx;
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
        
        int i{ 0 }; // for putting vertices into gridMesh
        int j{ 0 }; // for putting indices into gridHighMesh
        int k{ 0 }; // for putting indices into gridNormMesh
        float gridMesh[numX * 6 + numY * 6]; // will contain the vertex data of the grid
        int gridNormIndex[(numX - numHighX) * 2 + (numY - numHighY) * 2]; // will contain the index data of the normal lines of the grid
        int gridHighIndex[numHighX * 2 + numHighY * 2]; // will contain the index data of the highlighted lines of the grid
        for (int x = grid_min_x; x <= grid_max_x; ++x) {
            gridMesh[i * 6] = (float)(x * m_gridScale);
            gridMesh[1 + i * 6] = (float)(grid_min_y * m_gridScale);
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = (float)(x * m_gridScale);
            gridMesh[4 + i * 6] = (float)(grid_max_y * m_gridScale);
            gridMesh[5 + i * 6] = 0.0f;
            if (x % highlighter == 0) { // symmetrical code: (highlighter != 1 and x % highlighter == 0) or (x == grid_min_x or x == grid_max_x)
                gridHighIndex[j * 2] = i * 2;
                gridHighIndex[1 + j * 2] = i * 2 + 1;
                ++j;
            } else {
                gridNormIndex[k * 2] = i * 2;
                gridNormIndex[1 + k * 2] = i * 2 + 1;
                ++k;
            }
            ++i;
        }
        for (int y = grid_min_y; y <= grid_max_y; ++y) {
            gridMesh[i * 6] = (float)(grid_min_x * m_gridScale);
            gridMesh[1 + i * 6] = (float)(y * m_gridScale);
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = (float)(grid_max_x * m_gridScale);
            gridMesh[4 + i * 6] = (float)(y * m_gridScale);
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

        float planeMesh[12] {
            (float)(grid_min_x * m_gridScale), (float)(grid_min_y * m_gridScale), 0.0f,
            (float)(grid_min_x * m_gridScale), (float)(grid_max_y * m_gridScale), 0.0f,
            (float)(grid_max_x * m_gridScale), (float)(grid_max_y * m_gridScale), 0.0f,
            (float)(grid_max_x * m_gridScale), (float)(grid_min_y * m_gridScale), 0.0f
        };
        int planeIndex[6]{
            0, 1, 2,
            2, 3, 0
        };
        glBindVertexArray(VAO[(int)vertex::plane]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::plane]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeMesh), planeMesh, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::plane]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndex), planeIndex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        m_planeIndexSize = sizeof(planeIndex);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

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

    // Draw plane
    if (planeAlpha > 0.0f) {
        glm::vec4 col{ (m_background[0] + m_foreground[0])/2, (m_background[1] + m_foreground[1])/2, (m_background[2] + m_foreground[2])/2, planeAlpha };
        m_shaderProgramCol.setVec4("u_color", col);
        glDepthMask(false);
        glBindVertexArray(VAO[(int)vertex::plane]);
        glDrawElements(GL_TRIANGLES, m_planeIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
        glDepthMask(true);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void Renderer::drawCube(glm::vec3 const& p, double d, std::vector<color> const& c) const {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getProjection() };
    glm::mat4 const& view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };
    model = glm::translate(model, p);
    model = glm::scale(model, glm::vec3(d));
    glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(view * model)));
    glm::vec4 col{ c[0].red(), c[0].green(), c[0].blue(), c[0].alpha() }; // access to first color only is temporary...
    
    // Draw cube
    m_shaderProgram.use();
    glBindVertexArray(VAO[(int)vertex::cube]);
    m_shaderProgram.setVec3("u_lightPos", m_lightPos);
    m_shaderProgram.setFloat("u_ambientStrength", 0.4f);
    m_shaderProgram.setVec4("u_objectColor", col);
    m_shaderProgram.setVec3("u_lightColor", LIGHT_COLOR);
    m_shaderProgram.setMat4("u_projection", projection);
    m_shaderProgram.setMat4("u_view", view);
    m_shaderProgram.setMat4("u_model", model);
    m_shaderProgram.setMat3("u_normal", normal);
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
    glm::mat4 const& projection{ m_camera.getProjection() };
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
    // activate corresponding render state	
    m_shaderProgramFont.use();
    m_shaderProgramFont.setVec3("u_textColor", m_foreground);
    m_shaderProgramFont.setInt("u_text", 0);
    m_shaderProgramFont.setMat4("u_projection", glm::ortho(0.0f, (float)m_currentWidth, 0.0f, (float)m_currentHeight));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO[(int)vertex::font]);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        glyph ch = m_glyphs[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // update VBO for each character
        float vertices[24] = {
            xpos,     ypos + h,   0.0f, 0.0f,            
            xpos,     ypos,       0.0f, 1.0f,
            xpos + w, ypos,       1.0f, 1.0f,

            xpos,     ypos + h,   0.0f, 0.0f,
            xpos + w, ypos,       1.0f, 1.0f,
            xpos + w, ypos + h,   1.0f, 0.0f           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::font]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
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

unsigned int Renderer::getTextureID(std::string path) {
    unsigned int r{ 0 };   
    try {
        r = m_textures.at(path);
    } catch (const std::out_of_range& exception) {
        std::cerr << "ERROR::RENDERER::GET_TEXTURE_ID::TEXTURE_NOT_FOUND (" << path << ")\n";
    }
    
    return r;
}

GLFWwindow* Renderer::getWindow() {
    return m_window;
}

void Renderer::setDefaultCameraView(glm::vec3 position, float depth, glm::vec3 worldUp, float yaw, float pitch) {
    m_camera.setViewDefault(position, depth, worldUp, yaw, pitch);
}

void Renderer::setLightPosition(glm::vec3& newPos) {
    m_lightPos = newPos;
}

void Renderer::setGridScale(double newScale) {
    m_gridScale = newScale;
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, inputChannels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // store texture into map
        m_textures.insert(std::pair<std::string, unsigned int>(path, texture));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else std::cerr << "ERROR::RENDERER::STBIMAGE::TEXTURE_LOAD_FAILED (" << path << ")\n";
    stbi_image_free(data);

    return texture;
}

void Renderer::unloadTexture(std::string path) {
    try {
        glDeleteTextures(1, &m_textures.at(path));
        m_textures.erase(path);
    } catch (const std::out_of_range& exception) {
        std::cerr << "ERROR::RENDERER::TEXTURE::TEXTURE_NOT_FOUND (" << path << ")\n";
    }
}

void Renderer::mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods) {
    m_camera.mouseInput(x, y, xFirst, yFirst, type, mods);
}

void Renderer::keyboardInput(int key, bool first, float deltaTime, int mods) {
    // Process renderer's input
    /*no_op*/
    
    // Process camera's input
    m_camera.keyboardInput(key, first, deltaTime, mods);
}

void Renderer::viewportResize(int width, int height) {
    glViewport(0, 0, width, height);
    m_currentWidth = width;
    m_currentHeight = height;
	m_camera.setScreen(width, height);
}
