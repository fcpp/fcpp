// Copyright © 2023 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "lib/graphics/renderer.hpp"


// using namespace fcpp and fcpp::internal to prevent very verbose code...
using namespace fcpp;
using namespace fcpp::internal;


/* --- PRIVATE (NON-INTEGRAL) STATIC CONSTANTS --- */
#ifdef _WIN32
    std::string const renderer::VERTEX_PHONG_PATH{ ".\\shaders\\vertex_diff.glsl" };
    std::string const renderer::FRAGMENT_PHONG_PATH{ ".\\shaders\\fragment_diff.glsl" };
    std::string const renderer::VERTEX_COLOR_PATH{ ".\\shaders\\vertex_col.glsl" };
    std::string const renderer::FRAGMENT_COLOR_PATH{ ".\\shaders\\fragment_col.glsl" };
    std::string const renderer::VERTEX_TEXTURE_PATH{ ".\\shaders\\vertex_texture.glsl" };
    std::string const renderer::FRAGMENT_TEXTURE_PATH{ ".\\shaders\\fragment_texture.glsl" };
    std::string const renderer::VERTEX_FONT_PATH{ ".\\shaders\\vertex_font.glsl" };
    std::string const renderer::FRAGMENT_FONT_PATH{ ".\\shaders\\fragment_font.glsl" };
    std::string const renderer::VERTEX_LABEL_PATH{ ".\\shaders\\vertex_label.glsl" };
    std::string const renderer::FRAGMENT_LABEL_PATH{ ".\\shaders\\fragment_label.glsl" };
    std::string const renderer::FONT_PATH{ ".\\fonts\\hack\\Hack-Regular.ttf" };
    std::string const renderer::TEXTURE_PATH{ ".\\textures\\" };
#else
    std::string const renderer::VERTEX_PHONG_PATH{ "./shaders/vertex_diff.glsl" };
    std::string const renderer::FRAGMENT_PHONG_PATH{ "./shaders/fragment_diff.glsl" };
    std::string const renderer::VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" };
    std::string const renderer::FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" };
    std::string const renderer::VERTEX_TEXTURE_PATH{ "./shaders/vertex_texture.glsl" };
    std::string const renderer::FRAGMENT_TEXTURE_PATH{ "./shaders/fragment_texture.glsl" };
    std::string const renderer::VERTEX_FONT_PATH{ "./shaders/vertex_font.glsl" };
    std::string const renderer::FRAGMENT_FONT_PATH{ "./shaders/fragment_font.glsl" };
    std::string const renderer::VERTEX_LABEL_PATH{ "./shaders/vertex_label.glsl" };
    std::string const renderer::FRAGMENT_LABEL_PATH{ "./shaders/fragment_label.glsl" };
    std::string const renderer::FONT_PATH{ "./fonts/hack/Hack-Regular.ttf" };
    std::string const renderer::TEXTURE_PATH{ "./textures/" };
#endif
    glm::vec3 const renderer::LIGHT_DEFAULT_POS{ 0.0f, 0.0f, 0.0f };
    glm::vec3 const renderer::LIGHT_COLOR{ 1.0f, 1.0f, 1.0f };


/* --- PRIVATE STATIC VARIABLES --- */
int renderer::s_planeIndexSize{ 0 };
int renderer::s_gridNormIndexSize{ 0 };
int renderer::s_gridHighIndexSize{ 0 };
bool renderer::s_commonIsReady{ false };
bool renderer::s_gridIsReady{ false };
shapes renderer::s_shapes{};
unsigned int renderer::s_shapeVBO[(int)shape::SIZE];
unsigned int renderer::s_shadowVBO[(int)shape::SIZE];
unsigned int renderer::s_meshVBO[(int)vertex::SIZE];
unsigned int renderer::s_meshEBO[(int)index::SIZE];
std::unordered_map<char, glyph> renderer::s_glyphs{};
GLFWcursor* renderer::s_cursorArrow;
GLFWcursor* renderer::s_cursorCross;
GLFWcursor* renderer::s_cursorHand;


/* --- LOCAL FUNCTIONS --- */
namespace {
    inline glm::vec4 color_to_vec(color const& c) {
        return glm::make_vec4(c.rgba);
    }
}


/* --- PRIVATE STATIC FUNCTIONS --- */
void renderer::initializeCommon() {
    if (!s_commonIsReady) {
        // Mark common structures as ready
        s_commonIsReady = true;

        // Allocate glyphs' texture buffers
        allocateGlyphTextures();

        // Allocate VBOs and EBOs for meshes and shapes
        allocateShapeVertex();
        allocateMeshVertex();

        // Create cursor objects
        s_cursorArrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        s_cursorCross = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
        s_cursorHand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    }
}

unsigned int renderer::loadTexture(std::string path) {
    if (path == "") return 0;
    unsigned int texture{ 0 };
    stbi_set_flip_vertically_on_load(true);
    // Load texture data
    int width, height, nrChannels;
    unsigned char* data{ stbi_load((TEXTURE_PATH + path).c_str(), &width, &height, &nrChannels, 0) };
    if (data) {
        // Generate texture
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

bool renderer::unloadTexture(unsigned int id) {
    unsigned char isTexture{ glIsTexture(id) };
    bool success{ isTexture == GL_TRUE };
    glDeleteTextures(1, &id); // be aware: glDeleteTextures() silently ignores 0's and names that do not correspond to existing textures
    if (!success) std::cerr << "ERROR::RENDERER::TEXTURE::TEXTURE_NOT_FOUND (id = " << id << ")\n";

    return success;
}

void renderer::allocateGlyphTextures() {
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
        s_glyphs.insert(std::pair<char, glyph>(c, gl));
    }

    // Deallocate FreeType structures
    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLib);
}

void renderer::allocateShapeVertex() {
    // Generate VBOs for standard shapes
    glGenBuffers((int)shape::SIZE, s_shapeVBO);
    glGenBuffers((int)shape::SIZE, s_shadowVBO);

    for (int i = 0; i < (int)shape::SIZE; i++) {
        // Get actual shape
        shape sh{ (shape)i };

        // Allocate (static) shape VBO
        glBindBuffer(GL_ARRAY_BUFFER, s_shapeVBO[(int)sh]);
        glBufferData(GL_ARRAY_BUFFER, s_shapes[sh].data.size() * sizeof(float), s_shapes[sh].data.data(), GL_STATIC_DRAW);

        // Allocate (static) shadow VBO
        glBindBuffer(GL_ARRAY_BUFFER, s_shadowVBO[(int)sh]);
        glBufferData(GL_ARRAY_BUFFER, s_shapes[sh].shadow.size() * sizeof(float), s_shapes[sh].shadow.data(), GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderer::allocateMeshVertex() {
    // Generate VBOs and EBOs for general meshes
    glGenBuffers((int)vertex::SIZE, s_meshVBO);
    glGenBuffers((int)index::SIZE, s_meshEBO);

    // Allocate (dynamic) single line VBO
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::singleLine]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, NULL, GL_DYNAMIC_DRAW);

    // Allocate (dynamic) neighbour star VBO
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::star]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, NULL, GL_DYNAMIC_DRAW);

    // Unbind current VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/* --- CONSTRUCTOR --- */
renderer::renderer(size_t antialias, std::string name, bool master, GLFWwindow* masterPtr) :
    m_windowWidth{ SCR_DEFAULT_WIDTH },
    m_windowHeight{ SCR_DEFAULT_HEIGHT },
    m_renderScale{ 1.0 },
    m_master{ master },
    m_resizeOnSwap{ false },
    m_gridShow{ 3 },
    m_gridTexture{ 0 },
    m_lightPos{ LIGHT_DEFAULT_POS },
    m_background{ 1.0f, 1.0f, 1.0f, 1.0f },
    m_foreground{ 0.0f, 0.0f, 0.0f, 1.0f },
    m_rectangle_col{ 0.0f, 1.0f, 1.0f, 0.5f },
    m_camera{} {
    /* DEFINITION */
    // Initialize GLFW
    if (m_master) glfwInit();

    // Set context options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    if (antialias > 1)
        glfwWindowHint(GLFW_SAMPLES, antialias);

    // Create window (and its context)
    m_window = glfwCreateWindow(SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT, name.c_str(), NULL, masterPtr);

    if (m_window == NULL) {
        glfwTerminate();
        throw std::runtime_error("ERROR::RENDERER::GLFW::WINDOW_CREATION_FAILED\n");
    }

    // Initialize context if master
    if (m_master) initializeContext(true);
}


/* --- DESTRUCTOR --- */
renderer::~renderer() {
    if (m_window == NULL) return;

    // Destroy window and all associated resources (to do on main thread).
    glfwDestroyWindow(m_window);

    // Reset static members if master is closed
    if (m_master) {
        glfwTerminate();
        s_glyphs.erase(s_glyphs.begin(), s_glyphs.end()); // a new insertion is ignored if its key is already used
        s_gridIsReady = false;
        s_commonIsReady = false;
    }

    // Camera's, shapes' and shaders' destructors will be executed after this one.
}


/* --- PRIVATE FUNCTIONS --- */
int renderer::euclid(int a, int b) {
    int r;
    while(b != 0) //repeat until b is 0
    {
         r = a % b;
         a = b;
         b = r; //swap a and b
    }
    return a; //the result is a when b is equal to 0
}

void renderer::generateShapeAttributePointers() {
    // Generate VAOs and VBOs for standard shapes
    glGenVertexArrays((int)shape::SIZE, m_shapeVAO);
    glGenVertexArrays((int)shape::SIZE, m_shadowVAO);

    for (int i = 0; i < (int)shape::SIZE; i++) {
        // Get actual shape
        shape sh{ (shape)i };

        // Allocate shape attribute pointers
        glBindVertexArray(m_shapeVAO[(int)sh]);
        glBindBuffer(GL_ARRAY_BUFFER, s_shapeVBO[(int)sh]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Allocate shadow attribute pointers
        glBindVertexArray(m_shadowVAO[(int)sh]);
        glBindBuffer(GL_ARRAY_BUFFER, s_shadowVBO[(int)sh]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void renderer::generateMeshAttributePointers() {
    // Generate VAOs for general meshes
    glGenVertexArrays((int)vertex::SIZE, m_meshVAO);

    // Allocate single line attribute pointers
    glBindVertexArray(m_meshVAO[(int)vertex::singleLine]);
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::singleLine]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allocate neighbour star attribute pointers
    glBindVertexArray(m_meshVAO[(int)vertex::star]);
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::star]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Allocate rectangle attribute pointers
    glBindVertexArray(m_meshVAO[(int)vertex::rectangle]);
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::rectangle]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderer::generateFontBuffers() {
    // Generate VAO and VBO
    glGenVertexArrays(1, &m_fontVAO);
    glGenBuffers(1, &m_fontVBO);

    // Allocate (dynamic) font buffers
    glBindVertexArray(m_fontVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_fontVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Generate VAO and VBO
    glGenVertexArrays(1, &m_labelVAO);
    glGenBuffers(1, &m_labelVBO);

    // Allocate (dynamic) font buffers
    glBindVertexArray(m_labelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_labelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 5, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderer::generateShaderPrograms() {
    // Generate actual shader programs
    if (m_master) {
        m_shaderProgramDiff = shader{ VERTEX_PHONG_PATH.c_str(), FRAGMENT_PHONG_PATH.c_str() };
        m_shaderProgramCol = shader{ VERTEX_COLOR_PATH.c_str(), FRAGMENT_COLOR_PATH.c_str() };
        m_shaderProgramTexture = shader{ VERTEX_TEXTURE_PATH.c_str(), FRAGMENT_TEXTURE_PATH.c_str() };
        m_shaderProgramLabel = shader{ VERTEX_LABEL_PATH.c_str(), FRAGMENT_LABEL_PATH.c_str() };
    }
    m_shaderProgramFont = shader{ VERTEX_FONT_PATH.c_str(), FRAGMENT_FONT_PATH.c_str() };
}


/* --- PUBLIC FUNCTIONS --- */
void renderer::initializeContext(bool master) {
    // Set window's context as thread's current
    glfwMakeContextCurrent(m_window);

    // Initialize GLAD
    if (master and !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("ERROR::RENDERER::GLAD::INIT_FAILED\n");

    // Enabling V-Sync
    glfwSwapInterval(1);

    // Enabling depth test
    glEnable(GL_DEPTH_TEST);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable antialiasing
    glEnable(GL_MULTISAMPLE);

    // Initialize common resources
    initializeCommon();

    // Generate VAO for meshes and shapes
    generateMeshAttributePointers();
    generateShapeAttributePointers();

    // Generate VAO and VBO for fonts
    generateFontBuffers();

    // Generate shader programs
    generateShaderPrograms();

    // Set initial window metrics
    glfwGetFramebufferSize(m_window, (int*)&m_framebufferWidth, (int*)&m_framebufferHeight);
    m_renderScale = m_framebufferWidth / m_windowWidth;
    m_camera.setScreen(m_framebufferWidth, m_framebufferHeight);
    glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);

    // Clear first frame
    glClearColor(m_background[0], m_background[1], m_background[2], m_background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer::swapAndNext() {
    // Swap double buffers, check and call events
    glfwSwapBuffers(m_window);
    if (m_master) glfwPollEvents();
    else if (m_resizeOnSwap) {
        glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);
        m_resizeOnSwap = false;
    }

    // Clear frame
    glClearColor(m_background[0], m_background[1], m_background[2], m_background[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer::makeGrid(glm::vec3 gridMin, glm::vec3 gridMax, double gridScale) {
    // Initialize grid and plane buffers if they are not already
    if (!s_gridIsReady) {
        // The grid is initiated
        s_gridIsReady = true;

        // Calculate data for mesh generation
        int approx{ (gridMax.x - gridMin.x) * (gridMax.y - gridMin.y) > 2000 * gridScale * gridScale ? 10 : 1 };
        int grid_min_x = std::ceil(gridMin.x / gridScale / approx) * approx;
        int grid_max_x = std::floor(gridMax.x / gridScale / approx) * approx;
        int grid_min_y = std::ceil(gridMin.y / gridScale / approx) * approx;
        int grid_max_y = std::floor(gridMax.y / gridScale / approx) * approx;
        int numX{ grid_max_x - grid_min_x + 1 };
        int numY{ grid_max_y - grid_min_y + 1 };

        int highlighter{ 10 }; // the module required by a line to be highlighted

        int numHighX{ (grid_max_x - grid_max_x % highlighter - grid_min_x) / highlighter + 1 };
        int numHighY{ (grid_max_y - grid_max_y % highlighter - grid_min_y) / highlighter + 1 };

        // Generating grid mesh
        int i{ 0 }; // for putting vertices into gridMesh
        int j{ 0 }; // for putting indices into gridHighMesh
        int k{ 0 }; // for putting indices into gridNormMesh
        float gridMesh[numX * 6 + numY * 6]; // will contain the vertex data of the grid
        int gridNormIndex[(numX - numHighX) * 2 + (numY - numHighY) * 2]; // will contain the index data of the normal lines of the grid
        int gridHighIndex[numHighX * 2 + numHighY * 2]; // will contain the index data of the highlighted lines of the grid
        for (int x = grid_min_x; x <= grid_max_x; ++x) {
            gridMesh[0 + i * 6] = (float)(x * gridScale);
            gridMesh[1 + i * 6] = gridMin.y;
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = (float)(x * gridScale);
            gridMesh[4 + i * 6] = gridMax.y;
            gridMesh[5 + i * 6] = 0.0f;
            if (x % highlighter == 0) {
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
            gridMesh[0 + i * 6] = gridMin.x;
            gridMesh[1 + i * 6] = (float)(y * gridScale);
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = gridMax.x;
            gridMesh[4 + i * 6] = (float)(y * gridScale);
            gridMesh[5 + i * 6] = 0.0f;
            if (y % highlighter == 0) {
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
        glBindVertexArray(m_meshVAO[(int)vertex::grid]);
        glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::grid]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gridMesh), gridMesh, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::gridHigh]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridHighIndex), gridHighIndex, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::gridNorm]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridNormIndex), gridNormIndex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        s_gridNormIndexSize = sizeof(gridNormIndex);
        s_gridHighIndexSize = sizeof(gridHighIndex);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // Generating plane mesh
        float planeMesh[20]{
            // vertex coords                                                        // texture coords
            gridMin.x, gridMin.y, 0.0f, 0.0f, 0.0f,
            gridMin.x, gridMax.y, 0.0f, 0.0f, 1.0f,
            gridMax.x, gridMax.y, 0.0f, 1.0f, 1.0f,
            gridMax.x, gridMin.y, 0.0f, 1.0f, 0.0f
        };
        int planeIndex[6]{
            0, 1, 2,
            2, 3, 0
        };

        // Storing plane mesh
        glBindVertexArray(m_meshVAO[(int)vertex::plane]);
        glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::plane]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeMesh), planeMesh, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::plane]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndex), planeIndex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        s_planeIndexSize = sizeof(planeIndex);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void renderer::drawGrid(float planeAlpha) const {
    if (s_gridIsReady) {
        // Create matrices (used several times)
        glm::mat4 const& projection{ m_camera.getPerspective() };
        glm::mat4 const& view{ m_camera.getView() };
        glm::mat4 model{ 1.0f };

        if ((m_gridShow & 1) == 1) {
            // Set up shader program
            m_shaderProgramCol.use();
            m_shaderProgramCol.setMat4("u_projection", projection);
            m_shaderProgramCol.setMat4("u_view", view);
            m_shaderProgramCol.setMat4("u_model", model);

            // Draw grid
            glBindVertexArray(m_meshVAO[(int)vertex::grid]);
            m_shaderProgramCol.setVec4("u_color", m_foreground);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::gridHigh]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
            glDrawElements(GL_LINES, s_gridHighIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
            m_shaderProgramCol.setVec4("u_color", m_background);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::gridNorm]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
            glDrawElements(GL_LINES, s_gridNormIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
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
            glBindVertexArray(m_meshVAO[(int)vertex::plane]);
            glDrawElements(GL_TRIANGLES, s_planeIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
            glDepthMask(true);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}

void renderer::drawShape(shape sh, glm::vec3 const& p, double d, std::vector<color> const& c) const {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getPerspective() };
    glm::mat4 const& view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };
    model = glm::translate(model, p);
    model = glm::scale(model, glm::vec3(d));
    glm::mat3 normal{ glm::transpose(glm::inverse(view * model)) };

    // Draw shape
    m_shaderProgramDiff.use();
    m_shaderProgramDiff.setVec3("u_lightPos", m_lightPos);
    m_shaderProgramDiff.setFloat("u_ambientStrength", 0.4f);
    m_shaderProgramDiff.setVec3("u_lightColor", LIGHT_COLOR);
    m_shaderProgramDiff.setMat4("u_projection", projection);
    m_shaderProgramDiff.setMat4("u_view", view);
    m_shaderProgramDiff.setMat4("u_model", model);
    m_shaderProgramDiff.setMat3("u_normal", normal);
    glBindVertexArray(m_shapeVAO[(int)sh]);

    switch (c.size()) {
    default:
    case 1:
        m_shaderProgramDiff.setVec4("u_objectColor", color_to_vec(c[0]));
        glDrawArrays(GL_TRIANGLES, 0, s_shapes[sh].size[3]);
        break;
    case 2:
        m_shaderProgramDiff.setVec4("u_objectColor", color_to_vec(c[1]));
        glDrawArrays(GL_TRIANGLES, 0, s_shapes[sh].size[2]);
        m_shaderProgramDiff.setVec4("u_objectColor", color_to_vec(c[0]));
        glDrawArrays(GL_TRIANGLES, s_shapes[sh].size[2], s_shapes[sh].size[3] - s_shapes[sh].size[2]);
        break;
    case 3:
        for (int i = 0; i < 3; i++) {
            m_shaderProgramDiff.setVec4("u_objectColor", color_to_vec(c[2 - i]));
            glDrawArrays(GL_TRIANGLES, s_shapes[sh].size[i], s_shapes[sh].size[i+1] - s_shapes[sh].size[i]);
        }
        break;
    }

    // Draw pin
    if (p.z > 0 and (m_gridShow & 2) == 2) {
        float pinData[] = {
            p.x, p.y, p.z,
            p.x, p.y, 0.0f
        };
        m_shaderProgramCol.use();
        m_shaderProgramCol.setMat4("u_projection", projection);
        m_shaderProgramCol.setMat4("u_view", view);
        m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
        m_shaderProgramCol.setVec4("u_color", color_to_vec(c[0]));
        glBindVertexArray(m_meshVAO[(int)vertex::singleLine]);
        glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::singleLine]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(pinData), pinData);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_LINES, 0, 2);
    }
}

void renderer::drawShadow(shape sh, glm::vec3 p, double d, color const& c) const {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getPerspective() };
    glm::mat4 const& view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };
    p.z = 0.001f;
    model = glm::translate(model, p);
    model = glm::scale(model, glm::vec3(d));
    glm::mat3 normal{ glm::transpose(glm::inverse(view * model)) };

    // Draw shape
    m_shaderProgramDiff.use();
    m_shaderProgramDiff.setVec3("u_lightPos", m_lightPos);
    m_shaderProgramDiff.setFloat("u_ambientStrength", 0.4f);
    m_shaderProgramDiff.setVec3("u_lightColor", LIGHT_COLOR);
    m_shaderProgramDiff.setMat4("u_projection", projection);
    m_shaderProgramDiff.setMat4("u_view", view);
    m_shaderProgramDiff.setMat4("u_model", model);
    m_shaderProgramDiff.setMat3("u_normal", normal);
    glBindVertexArray(m_shadowVAO[(int)sh]);
    m_shaderProgramDiff.setVec4("u_objectColor", color_to_vec(c));
    glDrawArrays(GL_TRIANGLES, 0, s_shapes[sh].shadow.size());
}

//! @brief It draws a star of lines, given the center and sides.
void renderer::drawStar(glm::vec3 const& p, std::vector<glm::vec3> const& np) const {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getPerspective() };
    glm::mat4 const& view{ m_camera.getView() };

    float starData[6 * np.size()];
    for (int i = 0; i < np.size(); ++i) {
        starData[6 * i + 0] = p[0];
        starData[6 * i + 1] = p[1];
        starData[6 * i + 2] = p[2];
        starData[6 * i + 3] = np[i][0];
        starData[6 * i + 4] = np[i][1];
        starData[6 * i + 5] = np[i][2];
    }

    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);
    m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
    m_shaderProgramCol.setVec4("u_color", glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
    glBindVertexArray(m_meshVAO[(int)vertex::star]);
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::star]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(starData), starData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_LINES, 0, 2 * np.size());
}

//! @brief It draws the tail of a node, as a sequence of lines given their endpoints, the color to be used and a width.
void renderer::drawTail(std::deque<glm::vec3> const& p, std::deque<vec<2>> const& n, color const& c, float w) const {
    glm::vec4 col = color_to_vec(c);
    col.a /= 2;
    // Create matrices (used several times)
    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", m_camera.getPerspective());
    m_shaderProgramCol.setMat4("u_view", m_camera.getView());
    m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
    m_shaderProgramCol.setVec4("u_color", col);

    float tailVertices[6 * p.size()];
    for (int i = 0; i < p.size(); ++i) {
        float k = i/(p.size()-1.0f);
        k *= k * w * 0.62f;
        tailVertices[6 * i + 0] = p[i][0] + n[i][0] * k;
        tailVertices[6 * i + 1] = p[i][1] + n[i][1] * k;
        tailVertices[6 * i + 2] = p[i][2];
        tailVertices[6 * i + 3] = p[i][0] - n[i][0] * k;
        tailVertices[6 * i + 4] = p[i][1] - n[i][1] * k;
        tailVertices[6 * i + 5] = p[i][2];
    }
    unsigned int tailIndices[6 * p.size() - 6];
    for (int i = 0; i < p.size()-1; ++i) {
        tailIndices[6 * i + 0] = 2*i;
        tailIndices[6 * i + 1] = 2*i + 1;
        tailIndices[6 * i + 2] = 2*i + 2;
        tailIndices[6 * i + 3] = 2*i + 1;
        tailIndices[6 * i + 4] = 2*i + 2;
        tailIndices[6 * i + 5] = 2*i + 3;
    }

    glBindVertexArray(m_meshVAO[(int)vertex::rectangle]);
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::rectangle]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tailVertices), tailVertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::rectangle]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tailIndices), tailIndices, GL_DYNAMIC_DRAW);
    glDrawElements(GL_TRIANGLES, sizeof(tailIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderer::drawRectangle(float x, float y, float a, float b) const {
    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", glm::mat4{ 1.0f });
    m_shaderProgramCol.setMat4("u_view", glm::mat4{ 1.0f });
    m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
    m_shaderProgramCol.setVec4("u_color", m_rectangle_col);

    float vertices[] = {
        x, y, 0.0f,  // top right
        x, b, 0.0f,  // bottom right
        a, b, 0.0f,  // bottom left
        a, y, 0.0f  // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glBindVertexArray(m_meshVAO[(int)vertex::rectangle]);
    glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::rectangle]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::rectangle]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderer::drawText(std::string text, float x, float y, float scale) const {
    // Scale coordinates to renderbuffer's size
    x *= m_renderScale;
    y *= m_renderScale;
    scale *= m_renderScale;

    // Activate corresponding render state
    m_shaderProgramFont.use();
    m_shaderProgramFont.setVec3("u_textColor", m_foreground);
    m_shaderProgramFont.setInt("u_text", 0);
    m_shaderProgramFont.setMat4("u_projection", m_camera.getOrthographic());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_fontVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        glyph ch = s_glyphs[*c];

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
        glBindBuffer(GL_ARRAY_BUFFER, m_fontVBO);
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

void renderer::drawLabel(std::string text, glm::vec3 const& p, glm::vec4 col, float scale) const {
    // Activate corresponding render state
    m_shaderProgramLabel.use();
    m_shaderProgramLabel.setMat4("u_projection", m_camera.getPerspective());
    m_shaderProgramLabel.setMat4("u_view", m_camera.getView());
    m_shaderProgramLabel.setMat4("u_model", glm::mat4{ 1.0f });
    m_shaderProgramLabel.setVec4("u_color", col);
    m_shaderProgramLabel.setInt("u_texture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_labelVAO);

    // Iterate through all characters
    float x = p.x;
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        glyph ch = s_glyphs[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = p.y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // Update VBO for each character
        float vertices[30] = {
            xpos,     ypos + h,   p.z, 0.0f, 0.0f,
            xpos,     ypos,       p.z, 0.0f, 1.0f,
            xpos + w, ypos,       p.z, 1.0f, 1.0f,

            xpos,     ypos + h,   p.z, 0.0f, 0.0f,
            xpos + w, ypos,       p.z, 1.0f, 1.0f,
            xpos + w, ypos + h,   p.z, 1.0f, 0.0f
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, m_labelVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += ch.advance * scale / 64; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float renderer::getAspectRatio() {
    return (float)(m_windowWidth) / (float)(m_windowHeight);
}

int renderer::getWindowWidth() {
    return m_windowWidth;
}

int renderer::getWindowHeight() {
    return m_windowHeight;
}

int renderer::getFramebufferWidth() {
    return m_framebufferWidth;
}

int renderer::getFramebufferHeight() {
    return m_framebufferHeight;
}

double renderer::getRenderScale() {
    return m_renderScale;
}

GLFWwindow* renderer::getWindow() const {
    return m_window;
}

camera& renderer::getCamera() {
    return m_camera;
}

camera const& renderer::getCamera() const {
    return m_camera;
}

bool renderer::setGridTexture(std::string path) {
    bool success{ false };
    unsigned int loadedId{ loadTexture(path) };
    if (loadedId != 0) {
        if (m_gridTexture != 0) unloadTexture(m_gridTexture);
        m_gridTexture = loadedId;
        success = true;
    }

    return success;
}

void renderer::setLightPosition(glm::vec3& newPos) {
    m_lightPos = newPos;
}

void renderer::setStandardCursor(bool selection, bool setNow) {
    if (selection) m_standardCursor = s_cursorCross;
    else m_standardCursor = s_cursorArrow;

    if (setNow) glfwSetCursor(m_window, m_standardCursor);
}

void renderer::mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods) {
    // Set proper cursor shape
    if (type == mouse_type::click) {
        if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) glfwSetCursor(m_window, s_cursorHand);
        else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) glfwSetCursor(m_window, m_standardCursor);
    }

    // Call camera's mouse input handler
    m_camera.mouseInput(x, y, xFirst, yFirst, type, mods);
}

bool renderer::keyboardInput(int key, bool first, float deltaTime, int mods) {
    // Process renderer's input
    switch (key) {
        // show/hide grid
        case GLFW_KEY_G:
            if (first) m_gridShow = (m_gridShow+1) & 3;
            return true;
    }

    // Process camera's input
    return m_camera.keyboardInput(key, first, deltaTime, mods);
}

void renderer::viewportResize(int winWidth, int winHeight, int fbWidth, int fbHeight) {
    if (m_master) glViewport(0, 0, fbWidth, fbHeight);
    else m_resizeOnSwap = true;
    m_windowWidth = winWidth;
    m_windowHeight = winHeight;
    m_framebufferWidth = fbWidth;
    m_framebufferHeight = fbHeight;
    m_renderScale = fbWidth / winWidth;
    m_camera.setScreen(fbWidth, fbHeight);
}

void renderer::setColorTheme(color background, color foreground, color selection) {
    m_background = {background.rgba[0], background.rgba[1], background.rgba[2], background.rgba[3]};
    m_foreground = {foreground.rgba[0], foreground.rgba[1], foreground.rgba[2], foreground.rgba[3]};
    m_rectangle_col = {selection.rgba[0], selection.rgba[1], selection.rgba[2], selection.rgba[3]*0.5f};
}
