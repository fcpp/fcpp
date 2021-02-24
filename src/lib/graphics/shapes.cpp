// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <cmath>
#include <cstddef>

#include "lib/graphics/shapes.hpp"


//! @brief Cube's vertex data. (TO REMOVE)
const float fcpp::internal::Shapes::VERTEX_CUBE[216] = {
    // positions           // normals         
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f
};


//! @brief Compute normals of every triangle.
void fcpp::internal::VertexData::normalize() {
    for (size_t i=0; i<data.size(); i+=18) {
        // start of triangle data
        float* v = data.data() + i;
        // vector subtraction
        for (int j=6; j<=12; j+=6) for (int k=0; k<3; ++k)
            data[j + k + 3] = data[j + k] - data[k];
        // cross product
        for (int j=3; j<6; ++j)
            data[j] = data[9+(j+1)%3] * data[15+(j+2)%3] - data[9+(j+2)%3] * data[15+(j+1)%3];
        // calculate norm
        float norm = 0;
        for (int j=3; j<6; ++j)
            norm += data[j]*data[j];
        norm = sqrt(norm);
        // check cross product sign
        float sign = 0;
        for (int j=0; j<3; ++j)
            sign += data[j]*data[j+3];
        if (sign < 0) norm = -norm;
        // normalise
        for (int j=3; j<6; ++j)
            data[j] /= norm;
        // propagate
        for (int j=6; j<=12; j+=6) for (int k=3; k<6; ++k)
            data[j + k] = data[k];
    }
}

//! @brief Adds symmetric triangles (with respect to the origin).
void fcpp::internal::VertexData::symmetrize() {
    // duplicate size
    size_t n = data.size();
    data.resize(n*2);
    // shift center area at end
    for (size_t i=size[1]; i<n; ++i)
        data[i+n] = data[i];
    // symmetrise border area
    for (size_t i=0; i<size[1]; ++i)
        data[size[1]+i] = -data[i];
    // symmetrise center area
    for (size_t i=size[1]; i<n; ++i)
        data[size[1]+i] = -data[i+n];
    // adjust sizes
    size[3] = 2*n;
    size[2] = 2*size[1];
    size[0] = 0;
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
    v.size[1] = v.data.size();
    // half center area
    for (size_t i=1; i<3; ++i)
        push_rectangle(v, 0.5f, i,
                          -0.25f, -0.50f,
                          +0.25f, +0.50f);
    // fill missing pieces
    v.normalize();
    v.symmetrize();
}
