// Copyright © 2020-2021 Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#ifndef FCPP_GRAPHICS_SHADER_H
#define FCPP_GRAPHICS_SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

class Shader
{
private:
    // Program ID
    unsigned int m_ID;

public:
    // Constructor // builds and reads the shader
    Shader();
    Shader(const char* vertexPath, const char* fragmentPath);

    // Copy constructor
    Shader(const Shader& source);

    // Move constructor
    Shader(Shader&& source);

    // Copy assignment
    Shader& operator=(const Shader& source);

    // Move assignment
    Shader& operator=(Shader&& source);

    // Destructor
    ~Shader();

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
