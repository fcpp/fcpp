// Copyright © 2021 Luigi Rapetta. All Rights Reserved.

#include <fstream>
#include <sstream>
#include <iostream>

#include "lib/graphics/shader.hpp"

shader::shader() : m_ID{ 0 } { }

shader::shader(char const* vertexPath, char const* fragmentPath)
{
    /* RETRIEVE SOURCE CODE */
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // Ensure ifstream can now throw exceptions
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer into stream
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // Close files
        vShaderFile.close();
        fShaderFile.close();
        // Get strings from streams
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
    }

    // Getting code from strings
    char const* vShaderCode{ vertexCode.c_str() };
    char const* fShaderCode{ fragmentCode.c_str() };

    /* COMPILE SHADERS */
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
    }

    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\n";
    }

    // Shader program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, vertex);
    glAttachShader(m_ID, fragment);
    glLinkProgram(m_ID);
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
    }

    // Delete shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

shader::shader(shader&& source) : m_ID{ source.m_ID } { source.m_ID = 0; }

shader& shader::operator=(shader&& source) {
    // Self-assignment detection
    if (&source != this) {
        m_ID = source.m_ID;
        source.m_ID = 0;
    }

    return *this;
}

shader::~shader() {
    if(m_ID != 0 and glfwGetCurrentContext() != NULL) glDeleteProgram(m_ID);
}

void shader::use() const
{
    glUseProgram(m_ID);
}

void shader::setBool(std::string const& name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value);
}

void shader::setInt(std::string const& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
}

void shader::setFloat(std::string const& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
}

void shader::setVec3(std::string const& name, glm::vec3 const& value) const
{
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void shader::setVec4(std::string const& name, glm::vec4 const& value) const
{
    glUniform4fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void shader::setMat3(std::string const& name, glm::mat3 const& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void shader::setMat4(std::string const& name, glm::mat4 const& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
