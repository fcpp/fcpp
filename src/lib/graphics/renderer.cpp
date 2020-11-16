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

#include "lib/graphics/camera.h"
#include "lib/graphics/renderer.h"
#include "lib/graphics/shader.h"
#include "lib/graphics/shapes.h"


// using namespace fcpp::internal to prevent very verbose code...
using namespace fcpp::internal;


/* --- PRIVATE (NON-INTEGRAL) CONSTANTS --- */
#ifdef _WIN32
    const std::string Renderer::VERTEX_PATH{ ".\\shaders\\vertex.glsl" };
    const std::string Renderer::FRAGMENT_PATH{ ".\\shaders\\fragment.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ ".\\shaders\\vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ ".\\shaders\\fragment_col.glsl" };
#else
    const std::string Renderer::VERTEX_PATH{ "./shaders/vertex.glsl" };
    const std::string Renderer::FRAGMENT_PATH{ "./shaders/fragment.glsl" };
    const std::string Renderer::VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" };
    const std::string Renderer::FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" };
#endif

    const glm::vec3 Renderer::LIGHT_DEFAULT_POS{ glm::vec3{0.0f, 0.0f, 0.0f} };


/* --- CONSTRUCTOR --- */
Renderer::Renderer() :
    m_currentWidth{ SCR_DEFAULT_WIDTH },
    m_currentHeight{ SCR_DEFAULT_HEIGHT },
    m_orthoSize{ SCR_DEFAULT_ORTHO },
    m_gridScale{ 1.0 },
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

    // Generate VAOs, VBOs and EBOs
    glGenVertexArrays(4, VAO);
    glGenBuffers(4, VBO);
    glGenBuffers(1, EBO);

    // Store line data
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::VERTEX_LINE), Shapes::VERTEX_LINE, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Store line data
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::VERTEX_SQUARE), Shapes::VERTEX_SQUARE, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]); // VAO[1] stores EBO[0] here; do NOT unbind EBO[0] until VAO[1] is unbound
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Shapes::INDEX_SQUARE), Shapes::INDEX_SQUARE, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Store ortho data
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Shapes::VERTEX_ORTHO), Shapes::VERTEX_ORTHO, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Store cube data
    glBindVertexArray(VAO[3]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
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
    //if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    //    glfwSetWindowShouldClose(m_window, true);
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
    glm::mat4 view{ m_camera.getViewMatrix() };
    glm::mat4 model;

    // Setting up commonly used things...
    float gridWidth = std::abs(gridMin.x) + std::abs(gridMax.x);
    float gridHeight = std::abs(gridMin.y) + std::abs(gridMax.y);
    glm::vec3 gridCenter{ (gridMin.x + gridMax.x) / 2.0f, (gridMin.y + gridMax.y) / 2.0f, 0.0f };
    m_shaderProgramCol.use();
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);

    // Draw grid lines
    glBindVertexArray(VAO[0]);
    m_shaderProgramCol.setFloat("u_alpha", 1.0f);
    for (float i = 0.0f; i <= gridWidth; i += m_gridScale) {
        model = glm::mat4{ 1.0f };
        model = glm::translate(model, glm::vec3(i + (gridCenter.x - gridWidth / 2.0f), gridCenter.y, 0.0f));
        model = glm::scale(model, glm::vec3((float)gridHeight));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_shaderProgramCol.setMat4("u_model", model);
        glDrawArrays(GL_LINES, 0, sizeof(Shapes::VERTEX_LINE) / sizeof(Shapes::VERTEX_LINE[0]));
    }
    for (float i = 0.0f; i <= gridHeight; i += m_gridScale) {
        model = glm::mat4{ 1.0f };
        model = glm::translate(model, glm::vec3(gridCenter.x, i + (gridCenter.y - gridHeight / 2.0f), 0.0f));
        model = glm::scale(model, glm::vec3((float)gridWidth));
        m_shaderProgramCol.setMat4("u_model", model);
        glDrawArrays(GL_LINES, 0, sizeof(Shapes::VERTEX_LINE) / sizeof(Shapes::VERTEX_LINE[0]));
    }

    std::cout << "gridMin = (" << gridMin.x << ", " << gridMin.y << ", " << gridMin.x << ")\n";
    std::cout << "gridMax = (" << gridMax.x << ", " << gridMax.y << ", " << gridMax.x << ")\n";
    std::cout << "gridWidth = " << gridWidth << "\n";
    std::cout << "gridHeight = " << gridHeight << "\n";

    // Draw grid plane
    if (planeAlpha > 0.0f) {
        glDepthMask(false);
        glBindVertexArray(VAO[1]);
        m_shaderProgramCol.setFloat("u_alpha", planeAlpha);
        model = glm::mat4{ 1.0f };
        model = glm::translate(model, glm::vec3(gridCenter.x, gridCenter.y, 0.0f));
        model = glm::scale(model, glm::vec3((float)gridWidth, (float)gridHeight, 0.0f));
        m_shaderProgramCol.setMat4("u_model", model);
        glDrawElements(GL_TRIANGLES, sizeof(Shapes::INDEX_SQUARE) / sizeof(Shapes::INDEX_SQUARE[0]), GL_UNSIGNED_INT, 0);
        glDepthMask(true);
    }
}

void Renderer::drawOrtho() {
    // Create matrices (used several times)
    glm::mat4 projection{ 1.0f };
    glm::mat4 view{ 1.0f };
    glm::mat4 model{ 1.0f };

    // Draw ortho
    glClear(GL_DEPTH_BUFFER_BIT); // Clean depth buffer, in order to draw on top of 3D objects
    m_shaderProgramCol.use();
    glBindVertexArray(VAO[2]);
    m_shaderProgramCol.setFloat("u_alpha", 1.0f);
    projection = glm::ortho(0.0f, (float)m_currentWidth, 0.0f, (float)m_currentHeight, -(float)m_orthoSize, (float)m_orthoSize);
    m_shaderProgramCol.setMat4("u_projection", projection);
    m_shaderProgramCol.setMat4("u_view", view);
    model = glm::translate(model, glm::vec3((float)m_currentWidth - ((float)m_orthoSize * 5.0f / 4.0f), (float)m_orthoSize * 5.0f / 4.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-m_camera.getPitch()), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_camera.getYaw() + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3((float)m_orthoSize));
    m_shaderProgramCol.setMat4("u_model", model);
    glDrawArrays(GL_LINES, 0, sizeof(Shapes::VERTEX_ORTHO) / sizeof(Shapes::VERTEX_ORTHO[0]));
}



void Renderer::drawCube(glm::vec3 p, double d, std::vector<color> c) {
    // Create matrices (used several times)
    glm::mat4 projection{ glm::perspective(glm::radians(m_camera.getFov()), (float)m_currentWidth / (float)m_currentHeight, m_zNear, m_zFar) };
    glm::mat4 view{ m_camera.getViewMatrix() };
    glm::mat4 model{ 1.0f };
    glm::mat3 normal;

    // Draw cube
    m_shaderProgram.use();
    glBindVertexArray(VAO[3]);
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

float Renderer::aspectRatio() {
    return (float)(m_currentWidth) / (float)(m_currentHeight);
}

float Renderer::viewAngle() {
    return m_camera.getFov();
}

void Renderer::setPosition(glm::vec3& newPos) {
    m_camera.setPosition(newPos);
}

void Renderer::setLightPosition(glm::vec3& newPos) {
    m_lightPos = newPos;
}

void Renderer::setGridScale(double newScale) {
    m_gridScale = newScale;
}

void Renderer::setYaw(float newYaw) {
    m_camera.setYaw(newYaw);
}

void Renderer::setPitch(float newPitch) {
    m_camera.setPitch(newPitch);
}

void Renderer::setFarPlane(float newFar) {
    m_zFar = newFar;
}

void Renderer::setNearPlane(float newNear) {
    m_zNear = newNear;
}