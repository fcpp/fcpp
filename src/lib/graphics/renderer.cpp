// Copyright © 2026 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "lib/graphics/renderer.hpp"


// using namespace fcpp and fcpp::internal to prevent very verbose code...
using namespace fcpp;
using namespace fcpp::internal;


/* --- PRIVATE (NON-INTEGRAL) STATIC CONSTANTS --- */
#ifdef _WIN32
    std::string const renderer::VERTEX_DIFF_PATH{ ".\\shaders\\vertex_diff.glsl" };
    std::string const renderer::FRAGMENT_DIFF_PATH{ ".\\shaders\\fragment_diff.glsl" };
    std::string const renderer::VERTEX_COLOR_PATH{ ".\\shaders\\vertex_col.glsl" };
    std::string const renderer::FRAGMENT_COLOR_PATH{ ".\\shaders\\fragment_col.glsl" };
    std::string const renderer::VERTEX_TEXTURE_PATH{ ".\\shaders\\vertex_texture.glsl" };
    std::string const renderer::FRAGMENT_TEXTURE_PATH{ ".\\shaders\\fragment_texture.glsl" };
    std::string const renderer::VERTEX_TEXT_PATH{ ".\\shaders\\vertex_text.glsl" };
    std::string const renderer::FRAGMENT_TEXT_PATH{ ".\\shaders\\fragment_text.glsl" };
    std::string const renderer::FONT_PATH{ ".\\fonts\\hack\\Hack-Regular.ttf" };
    std::string const renderer::TEXTURE_PATH{ ".\\textures\\" };
#else
    std::string const renderer::VERTEX_DIFF_PATH{ "./shaders/vertex_diff.glsl" };
    std::string const renderer::FRAGMENT_DIFF_PATH{ "./shaders/fragment_diff.glsl" };
    std::string const renderer::VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" };
    std::string const renderer::FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" };
    std::string const renderer::VERTEX_TEXTURE_PATH{ "./shaders/vertex_texture.glsl" };
    std::string const renderer::FRAGMENT_TEXTURE_PATH{ "./shaders/fragment_texture.glsl" };
    std::string const renderer::VERTEX_TEXT_PATH{ "./shaders/vertex_text.glsl" };
    std::string const renderer::FRAGMENT_TEXT_PATH{ "./shaders/fragment_text.glsl" };
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
std::array<glyph, CHAR_MAX+1> renderer::s_glyphs{};
double renderer::s_textScale{ 0 };
unsigned int renderer::s_fontAtlas{ 0 };
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
    if (s_commonIsReady) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, s_fontAtlas);
    } else {
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

void renderer::allocateGlyphTextures() {
    // Initialize FreeType
    FT_Library ftLib;
    if (FT_Init_FreeType(&ftLib))
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::LIB_INIT_FAILED\n");
    FT_Face ftFace;
    if (FT_New_Face(ftLib, FONT_PATH.c_str(), 0, &ftFace)) {
        FT_Done_FreeType(ftLib);
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::FONT_LOAD_FAILED (" + FONT_PATH + ")\n");
    }
    if (FT_Set_Pixel_Sizes(ftFace, 0, FONT_DEFAULT_SIZE)) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLib);
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::PIXEL_SIZE_FAILED\n");
    }
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    maxTextureSize = std::min(maxTextureSize, 65536);
    size_t atlasWidth = 64, atlasHeight = 64;
    unsigned int x = 1, y = 1, rowHeight = 0;
    // First pass: determine glyph metrics and atlas positions
    for (int c = 0; c < s_glyphs.size(); ++c) {
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::RENDERER::FREETYPE::GLYPH_LOAD_FAILED (" << char(c) << ")\n";
            continue;
        }
        const auto& bitmap = ftFace->glyph->bitmap;
        if (bitmap.width == 0) {
            s_glyphs[c] = {{0,0}, {0,0}, static_cast<unsigned int>(ftFace->glyph->advance.x), {0,0,0,0}};
            continue;
        }
        while (x + bitmap.width >= atlasWidth and atlasWidth * 2 <= maxTextureSize) atlasWidth *= 2;
        if (x + bitmap.width >= atlasWidth) {
            x = 1;
            y += rowHeight + 1;
            rowHeight = 0;
        }
        s_glyphs[c] = {
            {bitmap.width, bitmap.rows},
            {ftFace->glyph->bitmap_left, ftFace->glyph->bitmap_top},
            static_cast<unsigned int>(ftFace->glyph->advance.x),
            {x, y, x+bitmap.width, y+bitmap.rows}
        };
        x += bitmap.width + 1;
        rowHeight = std::max(rowHeight, bitmap.rows);
        s_textScale = std::max(s_textScale, static_cast<double>(ftFace->glyph->advance.x));
    }
    if (s_textScale <= 0) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLib);
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::NO_VALID_GLYPHS\n");
    }
    s_textScale = 64.0 / s_textScale;
    while (y + rowHeight >= atlasHeight) atlasHeight *= 2;
    if (atlasHeight > maxTextureSize) {
        FT_Done_Face(ftFace);
        FT_Done_FreeType(ftLib);
        throw std::runtime_error("ERROR::RENDERER::FREETYPE::FONT_TOO_BIG_FOR_TEXTURE\n");
    }
    // Allocate a zero-filled atlas
    std::vector<unsigned char> atlas(atlasWidth * atlasHeight, 0);
    for (int c = 0; c < s_glyphs.size(); ++c) {
        glyph& g = s_glyphs[c];
        if (g.size.x == 0) continue;
        if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
            FT_Done_Face(ftFace);
            FT_Done_FreeType(ftLib);
            throw std::runtime_error("ERROR::RENDERER::FREETYPE::SECOND_PASS_GLYPH_FAIL\n");
        }
        // Load the bitmap into the atlas
        const auto& bitmap = ftFace->glyph->bitmap;
        for (int row = 0; row < g.size.y; ++row) {
            const unsigned char* src = bitmap.buffer + (bitmap.pitch >= 0 ? row : g.size.y - 1 - row) * std::abs(bitmap.pitch);
            std::copy_n(src, g.size.x, atlas.data() + size_t(g.offsets.y + row) * atlasWidth + size_t(g.offsets.x));
        }
        // Normalize offsets given the final atlas size
        g.size += glm::ivec2(1,1);
        g.bearing -= glm::vec2(0.5f,0.5f);
        g.offsets.x = (g.offsets.x - 0.5f) / atlasWidth;
        g.offsets.y = (g.offsets.y - 0.5f) / atlasHeight;
        g.offsets.z = (g.offsets.z + 0.5f) / atlasWidth;
        g.offsets.w = (g.offsets.w + 0.5f) / atlasHeight;
    }
    // Upload the atlas
    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &s_fontAtlas);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_fontAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Cleanup
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
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
    glGenVertexArrays(1, &m_textVAO);
    glGenBuffers(1, &m_textVBO);
    m_textVBOcontent.resize(6 * 5 * 24);
    // Allocate (dynamic) font buffers
    glBindVertexArray(m_textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
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
        m_shaderProgramDiff = shader{ VERTEX_DIFF_PATH.c_str(), FRAGMENT_DIFF_PATH.c_str() };
        m_shaderProgramCol = shader{ VERTEX_COLOR_PATH.c_str(), FRAGMENT_COLOR_PATH.c_str() };
        m_shaderProgramTexture = shader{ VERTEX_TEXTURE_PATH.c_str(), FRAGMENT_TEXTURE_PATH.c_str() };
    }
    m_shaderProgramText = shader{ VERTEX_TEXT_PATH.c_str(), FRAGMENT_TEXT_PATH.c_str() };
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
                m_shaderProgramTexture.setBool("u_drawTexture", true);
            }
            m_shaderProgramTexture.setVec4("u_color", col);
            m_shaderProgramTexture.setInt("u_texture", 1);
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(pinData), pinData, GL_DYNAMIC_DRAW);
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
void renderer::drawStar(glm::vec3 const& p, std::vector<std::pair<glm::vec3, float>> const& np, std::vector<glm::vec3> const& nl, color const& c) const {
    // Create matrices (used several times)
    glm::mat4 const& projection{ m_camera.getPerspective() };
    glm::mat4 const& view{ m_camera.getView() };
    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);
    m_shaderProgramCol.setMat4("u_model", glm::mat4{ 1.0f });
    m_shaderProgramCol.setVec4("u_color", color_to_vec(c));

    if (nl.size()) {
        float starVertices[6 * nl.size()];
        for (int i = 0; i < nl.size(); ++i) {
            glm::vec3 q = (nl[i] + p) * 0.5f;
            starVertices[6 * i + 0] = p[0];
            starVertices[6 * i + 1] = p[1];
            starVertices[6 * i + 2] = p[2];
            starVertices[6 * i + 3] = q[0];
            starVertices[6 * i + 4] = q[1];
            starVertices[6 * i + 5] = q[2];
        }
        glBindVertexArray(m_meshVAO[(int)vertex::star]);
        glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::star]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), starVertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_LINES, 0, 2 * nl.size());
    }
    if (np.size()) {
        float starVertices[12 * np.size()];
        for (int i = 0; i < np.size(); ++i) {
            glm::vec2 n = glm::normalize(glm::vec2(p[1] - np[i].first[1], np[i].first[0] - p[0])) * np[i].second;
            glm::vec3 q = (np[i].first + p) * 0.5f;
            starVertices[12 * i +  0] = p[0] + n[0];
            starVertices[12 * i +  1] = p[1] + n[1];
            starVertices[12 * i +  2] = p[2];
            starVertices[12 * i +  3] = q[0] + n[0];
            starVertices[12 * i +  4] = q[1] + n[1];
            starVertices[12 * i +  5] = q[2];
            starVertices[12 * i +  6] = p[0] - n[0];
            starVertices[12 * i +  7] = p[1] - n[1];
            starVertices[12 * i +  8] = p[2];
            starVertices[12 * i +  9] = q[0] - n[0];
            starVertices[12 * i + 10] = q[1] - n[1];
            starVertices[12 * i + 11] = q[2];
        }
        unsigned int starIndices[6 * np.size()];
        for (int i = 0; i < np.size(); ++i) {
            starIndices[6 * i + 0] = 4*i;
            starIndices[6 * i + 1] = 4*i + 1;
            starIndices[6 * i + 2] = 4*i + 2;
            starIndices[6 * i + 3] = 4*i + 1;
            starIndices[6 * i + 4] = 4*i + 2;
            starIndices[6 * i + 5] = 4*i + 3;
        }
        glBindVertexArray(m_meshVAO[(int)vertex::rectangle]);
        glBindBuffer(GL_ARRAY_BUFFER, s_meshVBO[(int)vertex::rectangle]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(starVertices), starVertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_meshEBO[(int)index::rectangle]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(starIndices), starIndices, GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, sizeof(starIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
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

namespace details {
    inline void assign(std::vector<float>&, size_t&) {}
    template <typename... Ts>
    inline void assign(std::vector<float>& v, size_t& i, float x, Ts... xs) {
        v[i++] = x;
        assign(v, i, xs...);
    }
}

void renderer::drawTextLabel(std::string const& text, glm::mat4 const& proj, glm::vec3 const& pos, float scale, glm::vec4 const& col = {0,0,0,1}) const {
    if (text.empty()) return;
    // Iterate through all characters
    float cursor = pos.x;
    double fontScale = scale * s_textScale;
    size_t buf_len = 0;
    for (char c : text) {
        const glyph& ch = s_glyphs[c >= 0 ? c : '?'];
        if (ch.size.x > 0) {
            if (buf_len == m_textVBOcontent.size())
                m_textVBOcontent.resize(buf_len+30);
            float xpos = cursor + ch.bearing.x * fontScale;
            float ypos = pos.y - (ch.size.y - ch.bearing.y) * fontScale;
            float w = ch.size.x * fontScale;
            float h = ch.size.y * fontScale;
            details::assign(m_textVBOcontent, buf_len,
                xpos,     ypos + h,  pos.z, ch.offsets.x, ch.offsets.y,
                xpos,     ypos,      pos.z, ch.offsets.x, ch.offsets.w,
                xpos + w, ypos,      pos.z, ch.offsets.z, ch.offsets.w,

                xpos,     ypos + h,  pos.z, ch.offsets.x, ch.offsets.y,
                xpos + w, ypos,      pos.z, ch.offsets.z, ch.offsets.w,
                xpos + w, ypos + h,  pos.z, ch.offsets.z, ch.offsets.y
            );
        }
        // Advance cursor for next glyph (advance is number of 1/64 pixels)
        cursor += ch.advance * fontScale / 64;
    }
    if (buf_len == 0) return;
    // Activate corresponding render state
    m_shaderProgramText.use();
    m_shaderProgramText.setInt("u_texture", 0);
    m_shaderProgramText.setVec4("u_color", col);
    m_shaderProgramText.setMat4("u_projection", proj);
    glBindVertexArray(m_textVAO);
    // Update content of VBO memory and render
    glBindBuffer(GL_ARRAY_BUFFER, m_textVBO);
    glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(float), m_textVBOcontent.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, buf_len/5);
    // Cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

float renderer::getAspectRatio() const {
    return (float)(m_windowWidth) / (float)(m_windowHeight);
}

int renderer::getWindowWidth() const {
    return m_windowWidth;
}

int renderer::getWindowHeight() const {
    return m_windowHeight;
}

int renderer::getFramebufferWidth() const {
    return m_framebufferWidth;
}

int renderer::getFramebufferHeight() const {
    return m_framebufferHeight;
}

double renderer::getRenderScale() const {
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
    if (path == "") return false;
    stbi_set_flip_vertically_on_load(true);
    // Load texture data
    int width, height, nrChannels;
    unsigned char* data{ stbi_load((TEXTURE_PATH + path).c_str(), &width, &height, &nrChannels, 0) };
    if (data) {
        // Generate texture
        glGenTextures(1, &m_gridTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_gridTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLint inputChannels;
        if (nrChannels == 3) inputChannels = GL_RGB;
        else inputChannels = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, inputChannels, width, height, 0, inputChannels, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cerr << "ERROR::RENDERER::STBIMAGE::TEXTURE_LOAD_FAILED (" << path << ")\n";
    stbi_image_free(data);
    return m_gridTexture != 0;
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
        if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS or glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT)) glfwSetCursor(m_window, s_cursorHand);
        else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE or glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) glfwSetCursor(m_window, m_standardCursor);
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
