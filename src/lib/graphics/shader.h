// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to learnopengl.com for the original structure.

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader
{
private:
    // Program ID
    unsigned int ID;

public:
    // Constructor // builds and reads the shader
    Shader() = default;
    Shader(const char* vertexPath, const char* fragmentPath);

    // Use/Activate the shader
    void use();

    // Uniform functions
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
};

#endif