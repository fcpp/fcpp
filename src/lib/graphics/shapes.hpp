// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

#include <vector>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Supported shapes for representing nodes.
enum class shape { cube, sphere, SIZE };

//! @brief Supported pointers to vertex buffers.
enum class vertex { font, singleLine, star, plane, grid, cube, sphere, SIZE };

//! @brief Supported pointers to index buffers.
enum class index { plane, gridNorm, gridHigh, SIZE };


//! @brief Namespace containing objects of internal use.
namespace internal {


//! @brief Collection of vertices.
struct VertexData {
    //! @brief Raw data of triangles as points and normals.
    std::vector<float> data;

    //! @brief Index of the start of data for all colors.
    size_t size[4];

    //! @brief Pointer to the start of data for color `i`.
    inline float const* operator[](size_t i) const {
        return data.data() + size[i];
    }

    //! @brief Inserts a point in the raw data.
    inline void push_point(float x, float y, float z) {
        data.push_back(x);
        data.push_back(y);
        data.push_back(z);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
    }

    //! @brief Compute normals of every triangle.
    void normalize();

    //! @brief Adds symmetric triangles (with respect to the origin).
    void symmetrize();
};

//! @brief Class holding the collections of vertices for every shape.
class Shapes {
public:
    //! @brief Cube's vertex data. (TO REMOVE)
    static const float VERTEX_CUBE[216];

    //! @brief Constructor.
    inline Shapes() {
        cube(m_vertices[0]);
    }

    //! @brief Const access.
    inline VertexData const& operator[](shape s) const {
        return m_vertices[(size_t)s];
    }

  private:
    //! @brief Generates vertex data for a cube.
    void cube(VertexData&);

    //! @brief THe collection of vertices for every shape.
    VertexData m_vertices[(size_t)shape::SIZE];
};


}


}

#endif // FCPP_GRAPHICS_SHAPES_H_
