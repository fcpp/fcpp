// Copyright © 2021 Luigi Rapetta. All Rights Reserved.

/**
 * @file shader.hpp
 * @brief Implementation of the `shader` class.
 */

#ifndef FCPP_GRAPHICS_SHADER_H
#define FCPP_GRAPHICS_SHADER_H

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//! @brief Class managing a shader program.
class shader {
private:
    //! @brief Program ID.
    unsigned int m_ID;

public:
    //! @brief Default constructor.
    shader();

    //! @brief Constructor building and reading the shader.
    shader(const char* vertexPath, const char* fragmentPath);

    //! @brief Copy constructor.
    shader(shader const& source);

    //! @brief Move constructor.
    shader(shader&& source);

    //! @brief Copy assignment.
    shader& operator=(shader const& source);

    //! @brief Move assignment.
    shader& operator=(shader&& source);

    //! @brief Destructor.
    ~shader();

    //! @brief Use/Activate the shader.
    void use() const;

    //! @brief Uniform functions.
    //! @{
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    //! @}
};

#endif // FCPP_GRAPHICS_SHADER_H
