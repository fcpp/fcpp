// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "lib/data/color.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Color constructor from a packed integral RGBA value.
color::color(packed_color irgba) : color((irgba >> 24) & 255, (irgba >> 16) & 255, (irgba >> 8) & 255, irgba & 255) {}

//! @brief Builds a color from its HSVA representation (h maxes to 360, the rest is normalised).
color color::hsva(double h, double s, double v, double a) {
    h -= 360 * floor(h / 360);
    double c = s * v;
    double x = c * (1 - abs(fmod(h / 60.0, 2) - 1));
    double m = v - c;
    double r, g, b;
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
    return { r + m, g + m, b + m, a };
}

//! @brief Compares colors.
bool color::operator==(const color& o) const {
    for (int i=0; i<4; ++i) {
        if (abs(rgba[i] - o.rgba[i]) > 0.01) return false;
    }
    return true;
}


}
