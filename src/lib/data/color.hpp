// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#ifndef FCPP_DATA_COLOR_H_
#define FCPP_DATA_COLOR_H_

#include <cstdint>
#include <cmath>

#include <type_traits>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Color type as a packed integer, for usage in template parameters.
using packed_color = uint32_t;

//! @brief Builds a packed color from its RGB representation.
constexpr packed_color packed_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return ((uint32_t)r << 24) + ((uint32_t)g << 16) + ((uint32_t)b << 8) + a;
}

//! @brief Builds a packed color from its HSVA representation (h maxes to 360, the rest to 100).
constexpr packed_color packed_hsva(int h, int s, int v, int a = 100) {
    h %= 360;
    int k = h%120 - 60;
    int c = s*v;
    int x = c*(k > 0 ? 60 - k : 60 + k)/60;
    int m = v*100-c;
    int r = 0, g = 0, b = 0;
    if (h >= 0 and h < 60)
        r = c, g = x, b = 0;
    else if (h >= 60 and h < 120)
        r = x, g = c, b = 0;
    else if (h >= 120 and h < 180)
        r = 0, g = c, b = x;
    else if (h >= 180 and h < 240)
        r = 0, g = x, b = c;
    else if (h >= 240 and h < 300)
        r = x, g = 0, b = c;
    else
        r = c, g = 0, b = x;
    return packed_rgba((r+m)*255/10000, (g+m)*255/10000, (b+m)*255/10000, a*255/100);
}


//! @brief Color data for visualisation purposes.
struct color {
    //! @brief Default color (white).
    color() : rgba{ 1,1,1,1 } {}

    //! @brief Color constructor from RGBA values.
    template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    color(T r, T g, T b, T a = (std::is_integral<T>::value ? 255 : 1)) {
        float mx = std::is_integral<T>::value ? 255 : 1;
        rgba[0] = r / mx;
        rgba[1] = g / mx;
        rgba[2] = b / mx;
        rgba[3] = a / mx;
    }

    //! @brief Color constructor from a packed integral RGBA value.
    color(packed_color irgba);

    //! @brief Compares colors.
    bool operator==(const color& o) const;

    //! @brief Access to the red component.
    float& red() {
        return rgba[0];
    }

    //! @brief Const access to the red component.
    float const& red() const {
        return rgba[0];
    }

    //! @brief Access to the green component.
    float& green() {
        return rgba[1];
    }

    //! @brief Const access to the green component.
    float const& green() const {
        return rgba[1];
    }

    //! @brief Access to the blue component.
    float& blue() {
        return rgba[2];
    }

    //! @brief Const access to the blue component.
    float const& blue() const {
        return rgba[2];
    }

    //! @brief Access to the alpha component.
    float& alpha() {
        return rgba[3];
    }

    //! @brief Const access to the alpha component.
    float const& alpha() const {
        return rgba[3];
    }

    //! @brief Builds a color from its HSVA representation (h maxes to 360, the rest is normalised).
    static color hsva(double h, double s, double v, double a = 1);

    //! @brief The float RGBA components of the color.
    float rgba[4];
};


}

#endif // FCPP_DATA_COLOR_H_
