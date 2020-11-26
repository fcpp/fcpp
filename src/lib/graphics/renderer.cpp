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

#include "lib/graphics/camera.hpp"
#include "lib/graphics/renderer.hpp"
#include "lib/graphics/shader.hpp"
#include "lib/graphics/shapes.hpp"


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
#else
    const std::string Renderer::VERTEX_PATH{ "./shaders/vertex.glsl" };
    const std::string Renderer::FRAGMENT_PATH{ "./shaders/fragment.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" };
    const std::string Renderer::VERTEX_ORTHO_PATH{ "./shaders/vertex_ortho.glsl" };
    const std::string Renderer::FRAGMENT_ORTHO_PATH{ "./shaders/fragment_ortho.glsl" };
#endif

    const glm::vec3 Renderer::LIGHT_DEFAULT_POS{ glm::vec3{0.0f, 0.0f, 0.0f} };


/* --- CONSTRUCTOR --- */
Renderer::Renderer() :
    m_currentWidth{ SCR_DEFAULT_WIDTH },
    m_currentHeight{ SCR_DEFAULT_HEIGHT },
    m_orthoSize{ SCR_DEFAULT_ORTHO },
    m_gridScale{ 1.0 },
    m_gridFirst{ 1 },
    m_planeIndexSize{ 0 },
    m_gridHighIndexSize{ 0 },
    m_gridNormIndexSize{ 0 },
    m_lightPos{ LIGHT_DEFAULT_POS },
    m_camera{},
    m_mouseLastX{ (float)(SCR_DEFAULT_WIDTH / 2) },
    m_mouseLastY{ (float)(SCR_DEFAULT_HEIGHT / 2) },
    m_mouseFirst{ 1 },
    m_deltaTime{ 0.0f },
    m_lastFrame{ 0.0f },
    m_zFar{ 1.0f },
    m_zNear{ 1000.0f }{
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
    // Create window (and its context)
    m_window = glfwCreateWindow(SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT, "fcppGL", NULL, NULL);

    if (m_window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.\n");
    }

    // Set newly created window's context as current
    glfwMakeContextCurrent(m_window);

    // Associates this (the Renderer instance) to m_window
    glfwSetWindowUserPointer(m_window, this);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD.\n");

    // Set viewport
    glViewport(0, 0, SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT);

    // Enable cursor capture
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Generate actual shader programs
    m_shaderProgram = Shader{ VERTEX_PATH.c_str(), FRAGMENT_PATH.c_str() };
    m_shaderProgramCol = Shader{ VERTEX_COLOR_PATH.c_str(), FRAGMENT_COLOR_PATH.c_str() };
    m_shaderProgramOrtho = Shader{ VERTEX_ORTHO_PATH.c_str(), FRAGMENT_ORTHO_PATH.c_str() };

    // Generate VAOs, VBOs and EBOs
    glGenVertexArrays(5, VAO);
    glGenBuffers(5, VBO);
    glGenBuffers(5, EBO); //on hold...

    // Store (static) ortho data 
    glBindVertexArray(VAO[(int)vertex::ortho]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::ortho]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::VERTEX_ORTHO), Shapes::VERTEX_ORTHO, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Store (static) cube data 
    glBindVertexArray(VAO[(int)vertex::cube]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[(int)vertex::cube]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::VERTEX_CUBE), Shapes::VERTEX_CUBE, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enabling depth test
    glEnable(GL_DEPTH_TEST);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Uncomment this call to draw in wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Clear first frame
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set internal callbacks
    setInternalCallbacks();
}


/* --- PRIVATE FUNCTIONS --- */
void Renderer::processKeyboardInput() {
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera.processKeyboard(FORWARD, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera.processKeyboard(BACKWARD, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera.processKeyboard(LEFT, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera.processKeyboard(RIGHT, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
        m_camera.processKeyboard(FLY_UP, m_deltaTime);
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
        m_camera.processKeyboard(FLY_DOWN, m_deltaTime);
}

void Renderer::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (m_mouseFirst) {
        m_mouseLastX = (float)xpos;
        m_mouseLastY = (float)ypos;
        m_mouseFirst = false;
    }

    float xoffset{ (float)(xpos - m_mouseLastX) };
    float yoffset{ (float)(m_mouseLastY - ypos) }; // reversed since y-coordinates range from bottom to top
    m_mouseLastX = (float)xpos;
    m_mouseLastY = (float)ypos;

    m_camera.processMouseMovement(xoffset, yoffset);
}

void Renderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    m_camera.processMouseScroll((float)yoffset);
}

void Renderer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    m_currentWidth = width;
    m_currentHeight = height;
}

void Renderer::setInternalCallbacks() {
    // Set viewport callbacks
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        Renderer& rend = *((Renderer*)glfwGetWindowUserPointer(window)); // get the Renderer instance from window
        rend.framebufferSizeCallback(window, width, height);
        });

    // Enable cursor callbacks
    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
        Renderer& rend = *((Renderer*)glfwGetWindowUserPointer(window)); // get the Renderer instance from window
        rend.mouseCallback(window, xpos, ypos);
        });
    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
        Renderer& rend = *((Renderer*)glfwGetWindowUserPointer(window)); // get the Renderer instance from window
        rend.scrollCallback(window, xoffset, yoffset);
        });
}


/* --- PUBLIC FUNCTIONS --- */
void Renderer::swapAndNext() {
    // Check and call events, swap double buffers
    glfwPollEvents();
    glfwSwapBuffers(m_window);

    // Deltatime
    float currentFrame{ (float)glfwGetTime() };
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    // Input
    processKeyboardInput();

    // Clear frame
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawGrid(glm::vec3 gridMin, glm::vec3 gridMax, float planeAlpha) {
    // Create matrices (used several times)
    glm::mat4 projection{ glm::perspective(glm::radians(m_camera.getFov()), (float)m_currentWidth / (float)m_currentHeight, m_zNear, m_zFar) };
    glm::mat4 view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };;

    // Set up shader program
    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);
    m_shaderProgramCol.setMat4("u_model", model);

    // Initialize grid and plane buffers if they are not already
    if (m_gridFirst) {
        m_gridFirst = 0;
        int grid_min_x = std::floor(gridMin.x / m_gridScale);
        int grid_max_x = std::ceil(gridMax.x / m_gridScale);
        int grid_min_y = std::floor(gridMin.y / m_gridScale);
        int grid_max_y = std::ceil(gridMax.y / m_gridScale);
        int numX = grid_max_x - grid_min_x + 1;
        int numY = grid_max_y - grid_min_y + 1;
        int numHighX = numX / 10 + 1;
        int numHighY = numY / 10 + 1;
        int i = 0; // for putting vertices into gridMesh
        int j = 0; // for putting indices into gridHighMesh
        int k = 0; // for putting indices into gridNormMesh

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
            if (x % 10 == 0) {
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
            gridMesh[i * 6] = (float)(grid_min_x * m_gridScale);
            gridMesh[1 + i * 6] = (float)(y * m_gridScale);
            gridMesh[2 + i * 6] = 0.0f;
            gridMesh[3 + i * 6] = (float)(grid_max_x * m_gridScale);
            gridMesh[4 + i * 6] = (float)(y * m_gridScale);
            gridMesh[5 + i * 6] = 0.0f;
            if (y % 10 == 0) {
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
    m_shaderProgramCol.setVec4("u_color", glm::vec4{ 1.0f });
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::gridHigh]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
    glDrawElements(GL_LINES, m_gridHighIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
    m_shaderProgramCol.setVec4("u_color", glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[(int)index::gridNorm]); // VAO stores EBO here; unbinding EBO before VAO is unbound will result in VAO pointing to no EBO
    glDrawElements(GL_LINES, m_gridNormIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Draw plane
    if (planeAlpha > 0.0f) {
        m_shaderProgramCol.setVec4("u_color", glm::vec4{ 0.3f, 0.3f, 0.3f, planeAlpha });
        glDepthMask(false);
        glBindVertexArray(VAO[(int)vertex::plane]);
        glDrawElements(GL_TRIANGLES, m_planeIndexSize / sizeof(int), GL_UNSIGNED_INT, 0);
        glDepthMask(true);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

/*
void Renderer::drawOrtho() {
    // Create matrices (used several times)
    glm::mat4 projection{ 1.0f };
    glm::mat4 view{ 1.0f };
    glm::mat4 model{ 1.0f };

    // Draw ortho
    glClear(GL_DEPTH_BUFFER_BIT); // Clean depth buffer, in order to draw on top of 3D objects
    m_shaderProgramOrtho.use();
    glBindVertexArray(VAO[(int)vertex::ortho]);
    m_shaderProgramOrtho.setFloat("u_alpha", 1.0f);
    projection = glm::ortho(0.0f, (float)m_currentWidth, 0.0f, (float)m_currentHeight, -(float)m_orthoSize, (float)m_orthoSize);
    m_shaderProgramOrtho.setMat4("u_projection", projection);
    m_shaderProgramOrtho.setMat4("u_view", view);
    model = glm::translate(model, glm::vec3((float)m_currentWidth - ((float)m_orthoSize * 5.0f / 4.0f), (float)m_orthoSize * 5.0f / 4.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-m_camera.getPitch()), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_camera.getYaw() + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3((float)m_orthoSize));
    m_shaderProgramOrtho.setMat4("u_model", model);
    glDrawArrays(GL_LINES, 0, sizeof(Shapes::VERTEX_ORTHO) / sizeof(Shapes::VERTEX_ORTHO[0]));
}
*/

void Renderer::drawCube(glm::vec3 p, double d, std::vector<color> c) {
    // Create matrices (used several times)
    glm::mat4 projection{ glm::perspective(glm::radians(m_camera.getFov()), (float)m_currentWidth / (float)m_currentHeight, m_zNear, m_zFar) };
    glm::mat4 view{ m_camera.getView() };
    glm::mat4 model{ 1.0f };
    glm::mat3 normal;

    // Draw cube
    m_shaderProgram.use();
    glBindVertexArray(VAO[(int)vertex::cube]);
    m_shaderProgram.setVec3("u_lightPos", m_lightPos);
    m_shaderProgram.setFloat("u_ambientStrength", 0.1f);
    m_shaderProgram.setVec4("u_objectColor", glm::vec4{ c[0].red(), c[0].green(), c[0].blue(), c[0].alpha() }); // access to first color only is temporary...
    m_shaderProgram.setVec3("u_lightColor", glm::vec3{ 1.0f, 1.0f, 1.0f });
    m_shaderProgram.setMat4("u_projection", projection);
    m_shaderProgram.setMat4("u_view", view);
    model = glm::translate(model, p);
    model = glm::scale(model, glm::vec3(d));
    m_shaderProgram.setMat4("u_model", model);
    normal = glm::mat3(glm::transpose(glm::inverse(view * model)));
    m_shaderProgram.setMat3("u_normal", normal);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

float Renderer::getAspectRatio() {
    return (float)(m_currentWidth) / (float)(m_currentHeight);
}

float Renderer::getViewAngle() {
    return m_camera.getFov();
}

GLFWwindow* Renderer::getWindow() {
    return m_window;
}

void Renderer::setDefaultCameraView(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch) {
    m_camera.setViewDefault(position, worldUp, yaw, pitch);
}

void Renderer::setLightPosition(glm::vec3& newPos) {
    m_lightPos = newPos;
}

void Renderer::setGridScale(double newScale) {
    m_gridScale = newScale;
}

void Renderer::setFarPlane(float newFar) {
    m_zFar = newFar;
}

void Renderer::setNearPlane(float newNear) {
    m_zNear = newNear;
}
