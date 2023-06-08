// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file shapes.hpp
 * @brief Implementation of the `shapes` class holding supported shapes (see \ref fcpp::shape "shape") as triangle lists.
 */

#ifndef FCPP_GRAPHICS_SHAPES_H_
#define FCPP_GRAPHICS_SHAPES_H_

#include <vector>

#include "lib/data/shape.hpp"
#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of internal use.
namespace internal {


//! @brief Collection of vertices.
struct VertexData {
    //! @brief Raw data of triangles as points and normals for the shape.
    std::vector<float> data;

    //! @brief Raw data of triangles as points and normals for the shape shadow.
    std::vector<float> shadow;

    //! @brief Index of the start of data (as points) for all the three colors; size[3] corresponds to the total number of points.
    size_t size[4];

    //! @brief Pointer to the start of data for color `i`.
    inline float const* operator[](size_t i) const {
        return data.data() + size[i] * 6;
    }

    //! @brief Inserts a point in the raw data.
    inline void push_point(float x, float y, float z) {
        pusher(data, x, y, z);
    }

    //! @brief Inserts a point in the raw data.
    inline void push_point(vec<3> const& xs) {
        push_point(xs[0], xs[1], xs[2]);
    }

    //! @brief Inserts a point in the raw data.
    inline void push_shadow_point(float x, float y) {
        pusher(shadow, x, y, 0);
    }

    //! @brief Inserts a point in the raw data.
    inline void push_shadow_point(vec<3> const& xs) {
        push_shadow_point(xs[0], xs[1]);
    }

    //! @brief Compute normals of every triangle.
    void normalize();

    //! @brief Compute normals spherifying the object.
    void spherify(float);

    //! @brief Adds symmetric triangles (with respect to the origin).
    void symmetrize();

  private:
    inline void pusher(std::vector<float>& v, float x, float y, float z) {
        v.push_back(x);
        v.push_back(y);
        v.push_back(z);
        v.push_back(0);
        v.push_back(0);
        v.push_back(1);
    }
};

//! @brief Class holding the collections of vertices for every shape.
class shapes {
public:
    //! @brief Constructor.
    inline shapes() {
        tetr(m_vertices[(size_t)shape::tetrahedron]);
        cube(m_vertices[(size_t)shape::cube]);
        octa(m_vertices[(size_t)shape::octahedron]);
        dome(m_vertices[(size_t)shape::icosahedron], 1);
        dome(m_vertices[(size_t)shape::sphere], FCPP_SPHERICITY);
        star(m_vertices[(size_t)shape::star]);
    }

    //! @brief Const access.
    inline VertexData const& operator[](shape s) const {
        return m_vertices[(size_t)s];
    }

private:
    //! @brief Generates vertex data for a tetrahedron.
    void tetr(VertexData&);

    //! @brief Generates vertex data for a cube.
    void cube(VertexData&);

    //! @brief Generates vertex data for a octahedron.
    void octa(VertexData&);

    //! @brief Generates vertex data for a icosahedron.
    void dome(VertexData&, size_t);

    //! @brief Generates vertex data for a star.
    void star(VertexData&);

    //! @brief The collection of vertices for every shape.
    VertexData m_vertices[(size_t)shape::SIZE];
};


}


}

#endif // FCPP_GRAPHICS_SHAPES_H_
