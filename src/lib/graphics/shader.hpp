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
    shader(char const* vertexPath, char const* fragmentPath);

    //! @brief Copy constructor.
    shader(shader const&) = delete;

    //! @brief Move constructor.
    shader(shader&& source);

    //! @brief Copy assignment.
    shader& operator=(shader const&) = delete;

    //! @brief Move assignment.
    shader& operator=(shader&& source);

    //! @brief Destructor.
    ~shader();

    //! @brief Use/Activate the shader.
    void use() const;

    //! @brief Uniform functions.
    //! @{

    //! @brief Sets a boolean property.
    void setBool(std::string const& name, bool value) const;

    //! @brief Sets an integral property.
    void setInt(std::string const& name, int value) const;

    //! @brief Sets a floating point property.
    void setFloat(std::string const& name, float value) const;

    //! @brief Sets a vector property.
    void setVec3(std::string const& name, glm::vec3 const& value) const;

    //! @brief Sets a homogeneous vector property.
    void setVec4(std::string const& name, glm::vec4 const& value) const;

    //! @brief Sets a matrix property.
    void setMat3(std::string const& name, glm::mat3 const& mat) const;

    //! @brief Sets an homogeneous matrix property.
    void setMat4(std::string const& name, glm::mat4 const& mat) const;
    //! @}
};

#endif // FCPP_GRAPHICS_SHADER_H
