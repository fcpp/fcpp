// Copyright © 2021 Luigi Rapetta. All Rights Reserved.

/**
 * @file shader.hpp
 * @brief Implementation of the `Shader` class.
 */

#ifndef FCPP_GRAPHICS_SHADER_H
#define FCPP_GRAPHICS_SHADER_H

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class shader
{
private:
    // Program ID
    unsigned int m_ID;

public:
    // Constructor // builds and reads the shader
    shader();
    shader(const char* vertexPath, const char* fragmentPath);

    // Copy constructor
    shader(shader const& source);

    // Move constructor
    shader(shader&& source);

    // Copy assignment
    shader& operator=(shader const& source);

    // Move assignment
    shader& operator=(shader&& source);

    // Destructor
    ~shader();

    // Use/Activate the shader
    void use() const;

    // Uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};

#endif // FCPP_GRAPHICS_SHADER_H
