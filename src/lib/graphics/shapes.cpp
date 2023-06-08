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


namespace {
    //! @brief Pushes a face with vertices in alternated triangulation order.
    void push_face(VertexData& v, std::vector<vec<3>> vx) {
        for (size_t i=0; i+2<vx.size(); ++i) for (size_t j=0; j<3; ++j)
            v.push_point(vx[i+j]);
    }
}

//! @brief Generates vertex data for a tetrahedron.
void shapes::tetr(VertexData& v) {
    float sq2 = sqrt(2);
    float sq3 = sqrt(3);
    float sq6 = sq2*sq3;
    std::vector<vec<3>> vx = {
        {-1, -1/sq3, -1/sq6},
        {+1, -1/sq3, -1/sq6},
        {+0, +2/sq3, -1/sq6},
        {+0, +0/sq3, +3/sq6}
    };
    std::vector<vec<3>> vxc[3];
    for (size_t i=0; i<2; ++i) {
        std::vector<vec<3>> vxr = {
            vx[2],
            0.66*vx[2] + 0.34*vx[i],
            vx[3],
            0.66*vx[3] + 0.34*vx[i]
        };
        for (size_t j=0; j<2; ++j) {
            vxc[0].push_back(vxr[j+0]);
            vxc[0].push_back(vxr[j+1]);
            vxc[0].push_back(vxr[j+2]);
        }
        vxc[i+1].push_back(vx[i]);
        vxc[i+1].push_back(0.66*vx[2]+0.34*vx[i]);
        vxc[i+1].push_back(0.66*vx[3]+0.34*vx[i]);
    }
    for (size_t i=2; i<4; ++i) {
        for (size_t j=0; j<2; ++j)
            for (size_t k=0; k<2; ++k) {
                vxc[k*(j+1)].push_back(vx[k?j:i]);
                vxc[k*(j+1)].push_back(0.66*vx[i]+0.34*vx[j]);
                vxc[k*(j+1)].push_back(0.67*vx[j]+0.33*vx[1-j]);
            }
        vxc[0].push_back(vx[i]);
        vxc[0].push_back(0.67*vx[0]+0.33*vx[1]);
        vxc[0].push_back(0.67*vx[1]+0.33*vx[0]);
    }
    v.size[0] = 0;
    for (int i=2; i>=0; --i) {
        for (auto const& p : vxc[i])
            v.push_point(p);
        v.size[3-i] = v.data.size()/6;
    }
    // normalize volume
    float f = cbrt(0.75*sq2);
    for (float& x : v.data) x *= f;
    // fill missing pieces
    v.normalize();
    // add shape shadow
    for (int i=0; i<3; ++i)
        v.push_shadow_point(vx[i] * f);
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
void shapes::cube(VertexData& v) {
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
    // add shape shadow
    v.push_shadow_point(-0.5f, -0.5f);
    v.push_shadow_point(+0.5f, +0.5f);
    v.push_shadow_point(+0.5f, -0.5f);
    v.push_shadow_point(-0.5f, -0.5f);
    v.push_shadow_point(+0.5f, +0.5f);
    v.push_shadow_point(-0.5f, +0.5f);
}

//! @brief Generates vertex data for a octahedron.
void shapes::octa(VertexData& v) {
    // border area
    std::vector<vec<3>> vx = {
        {0, 1, 0},
        {0, 0, 1},
        {0,-1, 0},
        {0, 0,-1},
        {0, 1, 0},
        {1, 0, 0},
        {-1,0, 0}
    };
    for (size_t i=0; i<4; ++i) {
        v.push_point(vx[5]);
        v.push_point(0.65*vx[i+0] + 0.35*vx[5]);
        v.push_point(0.65*vx[i+1] + 0.35*vx[5]);
    }
    v.size[1] = v.data.size()/6;
    // half center area
    for (size_t i=0; i<4; ++i) {
        push_face(v, {
            vx[i],
            0.65*vx[i] + 0.35*vx[5],
            vx[i+1],
            0.65*vx[i+1] + 0.35*vx[5]
        });
    }
    // normalize volume
    float f = cbrt(0.75);
    for (float& x : v.data) x *= f;
    // fill missing pieces
    v.normalize();
    v.symmetrize();
    // add shape shadow
    for (int i=0; i<2; ++i) {
        v.push_shadow_point(vx[0] * f);
        v.push_shadow_point(vx[2] * f);
        v.push_shadow_point(vx[5+i] * f);
    }
}

namespace {
    //! @brief Cartesian coordinates from spherical coordinates.
    inline vec<3> spherepoint(vec<2> p) {
        return {sin(p[0]), cos(p[0])*cos(p[1]), cos(p[0])*sin(p[1])};
    }

    //! @brief Pushes a (subdivided) triangle.
    void push_triangles(VertexData& v, vec<2> as, vec<2> ae, vec<2> b, vec<2> c, size_t k) {
        std::vector<std::vector<vec<2>>> vx;
        float pi = 4*atan(1);
        for (size_t i=0; i<=k; ++i) {
            vx.emplace_back();
            float y = i*1.0/k;
            float x = 1-y;
            vec<2> vs = x*as + y*b;
            vec<2> ve = x*ae + y*c;
            vx.back().push_back(vs);
            for (size_t j=1; j<=i; ++j)
                vx.back().push_back((vs*(i-j) + j*ve)/i);
        }
        for (size_t i=0; i<k; ++i) {
            std::vector<vec<3>> vxr;
            for (size_t j=0; j<=i; ++j) {
                vxr.push_back(spherepoint(vx[i+1][j]));
                vxr.push_back(spherepoint(vx[i][j]));
            }
            vxr.push_back(spherepoint(vx[i+1].back()));
            push_face(v, vxr);
        }
    }
}

//! @brief Generates vertex data for a icosahedric dome.
void shapes::dome(VertexData& v, size_t k) {
    float le = atan(0.5);
    float lp = 2*atan(1);
    // border area
    for (size_t i=0; i<5; ++i)
        push_triangles(v,
                       {lp, (4*i+1)*lp/5},
                       {lp, (4*i+5)*lp/5},
                       {le, (4*i+1)*lp/5},
                       {le, (4*i+5)*lp/5},
                       k);
    v.size[1] = v.data.size()/6;
    // half center area
    for (size_t i=0; i<5; ++i)
        push_triangles(v,
                       {-le, (4*i+3)*lp/5},
                       {-le, (4*i+3)*lp/5},
                       {+le, (4*i+1)*lp/5},
                       {+le, (4*i+5)*lp/5},
                       k);
    // fill missing pieces
    float f = cbrt(3/lp/8);
    if (k > 1)
        v.spherify(f);
    else {
        for (float& x : v.data) x *= 0.7332886800270005;
        v.normalize();
    }
    v.symmetrize();
    // add shape shadow
    size_t sizes = 3 << k;
    double alpha = (atan(1) * 8) / sizes;
    for (int i=0; i<sizes; ++i) {
        v.push_shadow_point(0, 0);
        v.push_shadow_point(cos(alpha*i)*f, sin(alpha*i)*f);
        v.push_shadow_point(cos(alpha*(i+1))*f, sin(alpha*(i+1))*f);
    }
}

//! @brief Generates vertex data for a star.
void shapes::star(VertexData& v) {
    float sq05 = sqrt(0.5);
    // border area
    for (int z=-1; z<=1; z+=2) {
        std::vector<vec<3>> vx = {
            {0.5, -0.5, 0},
            {1,   +0,   z*sq05},
            {0.5, +0.5, 0}
        };
        vec<3> q{0, 0, z*sq05};
        for (auto& p : vx) v.push_point(p);
        for (int y=-1; y<=1; y+=2) {
            push_face(v, {
                vx[1],
                0.75*q + 0.25*vx[1],
                vx[y+1],
                0.5*vx[y+1] + 0.5*q,
                0.5*vx[y+1] + 0.5*make_vec(0, y, z*sq05),
                0.75*vx[y+1] + 0.25*make_vec(0.5, -0.5*y, 0)
            });
        }
    }
    v.size[1] = v.data.size()/6;
    // half center area
    for (int z=-1; z<=1; z+=2) {
        std::vector<vec<3>> vx = {
            {-0.25, 0.5, 0},
            {+0,    1,   z*sq05},
            {+0.25, 0.5, 0}
        };
        vec<3> q{0, 0, z*sq05};
        for (auto& p : vx) v.push_point(p);
        for (int x=-1; x<=1; x+=2) {
            vec<3> p{x*real_t(0.5), real_t(0.5), 0};
            push_face(v, {
                0.75*q + 0.25*make_vec(x, 0, z*sq05),
                0.5*q + 0.5*p,
                q,
                0.5*vx[1] + 0.5*p,
                vx[1],
                vx[x+1]
            });
        }
    }
    // normalize volume
    float f = cbrt(sq05);
    for (float& x : v.data) x *= f;
    // fill missing pieces
    v.normalize();
    v.symmetrize();
    // add shape shadow
    std::vector<vec<3>> vx = {
        {0,    0,    0},
        {0.25, 0.25, 0},
        {0,    1,    0},
        {1,    0,    0}
    };
    f = cbrt(0.75);
    for (int x=-1; x<=1; x+=2)
        for (int y=-1; y<=1; y+=2)
            for (int i=0; i<2; ++i) {
                v.push_shadow_point(vx[0][0] * f * x, vx[0][1] * f * y);
                v.push_shadow_point(vx[1][0] * f * x, vx[1][1] * f * y);
                v.push_shadow_point(vx[2+i][0] * f * x, vx[2+i][1] * f * y);
            }
}


}

}
