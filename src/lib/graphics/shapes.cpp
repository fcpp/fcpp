// Copyright © 2021 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

#include <cmath>
#include <cstddef>

#include "lib/graphics/shapes.hpp"


namespace fcpp {

namespace internal {


//! @brief Compute normals of every triangle.
void VertexData::normalize() {
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

//! @brief Compute normals spherifying the object.
void VertexData::spherify(float radius) {
    for (size_t i=0; i<data.size(); i+=6) {
        // start of point data
        float* v = data.data() + i;
        // calculate norm
        float norm = 0;
        for (int j=0; j<3; ++j)
            norm += v[j]*v[j];
        norm = sqrt(norm);
        // normalise
        for (int j=0; j<3; ++j) {
            v[j+3] = v[j]/norm;
            v[j] = v[j+3]*radius;
        }
    }
}

//! @brief Adds symmetric triangles (with respect to the origin).
void VertexData::symmetrize() {
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


//! @brief Generates vertex data for a tetrahedron.
void Shapes::tetr(VertexData& v) {
    float sq2 = sqrt(2);
    float sq3 = sqrt(3);
    float sq6 = sq2*sq3;
    std::vector<vec<3>> vx = {
        {-1, -1/sq3, -1/sq6},
        {+1, -1/sq3, -1/sq6},
        {+0, +2/sq3, -1/sq6},
        {+0, +0/sq3, +3/sq6}
    };
    for (size_t i=0; i<4; ++i)
        for (size_t j=0; j<3; ++j) {
            v.push_point(vx[i]);
            v.push_point(0.5*(vx[i] + vx[(i+j+1)%4]));
            v.push_point(0.5*(vx[i] + vx[(i+(j+1)%3+1)%4]));
        }
    v.size[2] = v.data.size()/6;
    v.size[1] = v.size[2]/2;
    v.size[0] = 0;
    for (size_t i=0; i<4; ++i) {
        v.push_point(0.5*(vx[i] + vx[(i+1)%4]));
        v.push_point(0.5*(vx[i] + vx[(i+2)%4]));
        v.push_point(0.5*(vx[(i+1)%4] + vx[(i+2)%4]));
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
    void push_rectangle_point(VertexData& v, float z, size_t i, float x, float y) {
        switch (i) {
            case 0: v.push_point(z, x, y); break;
            case 1: v.push_point(x, z, y); break;
            case 2: v.push_point(x, y, z); break;
        }
    }

    //! @brief Pushes a rectangle as two triangles.
    void push_rectangle(VertexData& v, float z, size_t i, float x1, float y1, float x2, float y2) {
        push_rectangle_point(v, z, i, x1, y1);
        push_rectangle_point(v, z, i, x2, y2);
        push_rectangle_point(v, z, i, x2, y1);

        push_rectangle_point(v, z, i, x1, y1);
        push_rectangle_point(v, z, i, x2, y2);
        push_rectangle_point(v, z, i, x1, y2);
    }
}

//! @brief Generates vertex data for a cube.
void Shapes::cube(VertexData& v) {
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

namespace {
    //! @brief Cartesian coordinates from spherical coordinates.
    inline vec<3> spherepoint(float lat, float lon) {
        return {sin(lat), cos(lat)*cos(lon), cos(lat)*sin(lon)};
    }

    //! @brief Pushes a (subdivided) triangle.
    void push_triangles(VertexData& v, vec<3> a, vec<3> b, vec<3> c, size_t k) {
        std::vector<std::vector<vec<3>>> vx;
        for (size_t i=0; i<=k; ++i) {
            vx.emplace_back();
            float y = i*1.0/k;
            float x = 1-y;
            vec<3> vs = x*a + y*b;
            vec<3> ve = x*a + y*c;
            vx.back().push_back(vs);
            for (size_t j=1; j<=i; ++j)
                vx.back().push_back((vs*(i-j) + j*ve)/i);
        }
        for (size_t i=0; i<k; ++i) {
            std::vector<vec<3>> vxr;
            for (size_t j=0; j<=i; ++j) {
                vxr.push_back(vx[i+1][j]);
                vxr.push_back(vx[i][j]);
            }
            vxr.push_back(vx[i+1].back());
            for (size_t j=0; j<=2*i; ++j) {
                v.push_point(vxr[j+0]);
                v.push_point(vxr[j+1]);
                v.push_point(vxr[j+2]);
            }
        }
    }
}

//! @brief Generates vertex data for a icosahedric dome.
void Shapes::dome(VertexData& v, size_t k) {
    float le = atan(0.5);
    float lp = 2*atan(1);
    // border area
    for (size_t i=0; i<5; ++i)
        push_triangles(v,
                       spherepoint(lp, 0),
                       spherepoint(le, (4*i+1)*lp/5),
                       spherepoint(le, (4*i+5)*lp/5),
                       k);
    v.size[1] = v.data.size()/6;
    // half center area
    for (size_t i=0; i<5; ++i)
        push_triangles(v,
                       spherepoint(+le, (4*i+1)*lp/5),
                       spherepoint(-le, (4*i+3)*lp/5),
                       spherepoint(+le, (4*i+5)*lp/5),
                       k);
    // fill missing pieces
    if (k > 1)
        v.spherify(cbrt(3/lp/8));
    else {
        for (float& x : v.data) x *= 0.7332886800270005;
        v.normalize();
    }
    v.symmetrize();
}


}

}
