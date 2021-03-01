// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <cmath>
#include <cstddef>

#include "lib/graphics/shapes.hpp"


//! @brief Compute normals of every triangle.
void fcpp::internal::VertexData::normalize() {
    for (size_t i=0; i<data.size(); i+=18) {
        // start of triangle data
        float* v = data.data() + i;
        // vector subtraction
        for (int j=6; j<=12; j+=6) for (int k=0; k<3; ++k)
            v[j + k + 3] = v[j + k] - v[k];
        // cross product
        for (int j=3; j<6; ++j)
            v[j] = v[9+(j+1)%3] * v[15+(j+2)%3] - v[9+(j+2)%3] * v[15+(j+1)%3];
        // calculate norm
        float norm = 0;
        for (int j=3; j<6; ++j)
            norm += v[j]*v[j];
        norm = sqrt(norm);
        // check cross product sign
        float sign = 0;
        for (int j=0; j<3; ++j)
            sign += v[j]*v[j+3];
        if (sign < 0) norm = -norm;
        // normalise
        for (int j=3; j<6; ++j)
            v[j] /= norm;
        // propagate
        for (int j=6; j<=12; j+=6) for (int k=3; k<6; ++k)
            v[j + k] = v[k];
    }
}

//! @brief Adds symmetric triangles (with respect to the origin).
void fcpp::internal::VertexData::symmetrize() {
    // duplicate size
    size_t n = data.size();
    data.resize(n*2);
    // shift center area at end
    for (size_t i=size[1]*6; i<n; ++i)
        data[i+n] = data[i];
    // symmetrise border area
    for (size_t i=0; i<size[1]*6; ++i)
        data[size[1]*6+i] = -data[i];
    // symmetrise center area
    for (size_t i=size[1]*6; i<n; ++i)
        data[size[1]*6+i] = -data[i+n];
    // adjust sizes
    size[3] = n/3;
    size[2] = 2*size[1];
    size[0] = 0;
}


namespace {
    //! @brief Calculates the mid point between two points.
    inline std::vector<float> midpoint(std::vector<float> p, std::vector<float> q) {
        return {(p[0]+q[0])/2, (p[1]+q[1])/2, (p[2]+q[2])/2};
    }
}

//! @brief Generates vertex data for a cube.
void fcpp::internal::Shapes::tetrahedron(fcpp::internal::VertexData& v) {
    float sq2 = sqrt(2);
    float sq3 = sqrt(3);
    float sq6 = sq2*sq3;
    std::vector<std::vector<float>> vx = {
        {-1, -1/sq3, -1/sq6},
        {+1, -1/sq3, -1/sq6},
        {+0, +2/sq3, -1/sq6},
        {+0, +0/sq3, +3/sq6}
    };
    for (size_t i=0; i<4; ++i)
        for (size_t j=0; j<3; ++j) {
            v.push_point(vx[i]);
            v.push_point(midpoint(vx[i], vx[(i+j+1)%4]));
            v.push_point(midpoint(vx[i], vx[(i+(j+1)%3+1)%4]));
        }
    v.size[2] = v.data.size()/6;
    v.size[1] = v.size[2]/2;
    v.size[0] = 0;
    for (size_t i=0; i<4; ++i) {
        v.push_point(midpoint(vx[i], vx[(i+1)%4]));
        v.push_point(midpoint(vx[i], vx[(i+2)%4]));
        v.push_point(midpoint(vx[(i+1)%4], vx[(i+2)%4]));
    }
    v.size[3] = v.data.size()/6;
    // normalize volume
    float f = cbrt(0.75*sq2);
    for (float& x : v.data) x *= f;
    // fill missing pieces
    v.normalize();
}

namespace {
    //! @brief Pushes a point of a rectangle.
    void push_rectangle_point(fcpp::internal::VertexData& v, float z, size_t i, float x, float y) {
        switch (i) {
            case 0: v.push_point(z, x, y); break;
            case 1: v.push_point(x, z, y); break;
            case 2: v.push_point(x, y, z); break;
        }
    }

    //! @brief Pushes a rectangle as two triangles.
    void push_rectangle(fcpp::internal::VertexData& v, float z, size_t i, float x1, float y1, float x2, float y2) {
        push_rectangle_point(v, z, i, x1, y1);
        push_rectangle_point(v, z, i, x2, y2);
        push_rectangle_point(v, z, i, x2, y1);

        push_rectangle_point(v, z, i, x1, y1);
        push_rectangle_point(v, z, i, x2, y2);
        push_rectangle_point(v, z, i, x1, y2);
    }
}

//! @brief Generates vertex data for a cube.
void fcpp::internal::Shapes::cube(fcpp::internal::VertexData& v) {
    // border area
    push_rectangle(v, +0.50f, 0,
                      -0.50f, -0.50f,
                      +0.50f, +0.50f);
    for (float z=-0.5f; z<=0.5f; ++z)
        for (size_t i=1; i<3; ++i)
            push_rectangle(v, z, i,
                              +0.25f, -0.50f,
                              +0.50f, +0.50f);
    v.size[1] = v.data.size()/6;
    // half center area
    for (size_t i=1; i<3; ++i)
        push_rectangle(v, 0.5f, i,
                          -0.25f, -0.50f,
                          +0.25f, +0.50f);
    // fill missing pieces
    v.normalize();
    v.symmetrize();
}
