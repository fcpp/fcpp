// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file color.hpp
 * @brief Implementation of the `color` class.
 */

#ifndef FCPP_DATA_COLOR_H_
#define FCPP_DATA_COLOR_H_

#include <cstdint>
#include <cmath>

#include <string>
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

//! @brief HTML Colors.
//! @{
constexpr packed_color ALICE_BLUE             = 0xF0F8FFFF;
constexpr packed_color ANTIQUE_WHITE          = 0xFAEBD7FF;
constexpr packed_color AQUA                   = 0X00FFFFFF;
constexpr packed_color AQUAMARINE             = 0X7FFFD4FF;
constexpr packed_color AZURE                  = 0xF0FFFFFF;
constexpr packed_color BEIGE                  = 0xF5F5DCFF;
constexpr packed_color BISQUE                 = 0xFFE4C4FF;
constexpr packed_color BLACK                  = 0X000000FF;
constexpr packed_color BLANCHED_ALMOND        = 0xFFEBCDFF;
constexpr packed_color BLUE                   = 0X0000FFFF;
constexpr packed_color BLUE_VIOLET            = 0X8A2BE2FF;
constexpr packed_color BROWN                  = 0xA52A2AFF;
constexpr packed_color BURLY_WOOD             = 0xDEB887FF;
constexpr packed_color CADET_BLUE             = 0X5F9EA0FF;
constexpr packed_color CHARTREUSE             = 0X7FFF00FF;
constexpr packed_color CHOCOLATE              = 0xD2691EFF;
constexpr packed_color CORAL                  = 0xFF7F50FF;
constexpr packed_color CORNFLOWER_BLUE        = 0X6495EDFF;
constexpr packed_color CORNSILK               = 0xFFF8DCFF;
constexpr packed_color CRIMSON                = 0xDC143CFF;
constexpr packed_color CYAN                   = 0X00FFFFFF;
constexpr packed_color DARK_BLUE              = 0X00008BFF;
constexpr packed_color DARK_CYAN              = 0X008B8BFF;
constexpr packed_color DARK_GOLDENROD         = 0xB8860BFF;
constexpr packed_color DARK_GRAY              = 0xA9A9A9FF;
constexpr packed_color DARK_GREEN             = 0X006400FF;
constexpr packed_color DARK_KHAKI             = 0xBDB76BFF;
constexpr packed_color DARK_MAGENTA           = 0X8B008BFF;
constexpr packed_color DARK_OLIVE_GREEN       = 0X556B2FFF;
constexpr packed_color DARK_ORANGE            = 0xFF8C00FF;
constexpr packed_color DARK_ORCHID            = 0X9932CCFF;
constexpr packed_color DARK_RED               = 0X8B0000FF;
constexpr packed_color DARK_SALMON            = 0xE9967AFF;
constexpr packed_color DARK_SEA_GREEN         = 0X8FBC8BFF;
constexpr packed_color DARK_SLATE_BLUE        = 0X483D8BFF;
constexpr packed_color DARK_SLATE_GRAY        = 0X2F4F4FFF;
constexpr packed_color DARK_TURQUOISE         = 0X00CED1FF;
constexpr packed_color DARK_VIOLET            = 0X9400D3FF;
constexpr packed_color DEEP_PINK              = 0xFF1493FF;
constexpr packed_color DEEP_SKY_BLUE          = 0X00BFFFFF;
constexpr packed_color DIM_GRAY               = 0X696969FF;
constexpr packed_color DODGER_BLUE            = 0X1E90FFFF;
constexpr packed_color FIRE_BRICK             = 0xB22222FF;
constexpr packed_color FLORAL_WHITE           = 0xFFFAF0FF;
constexpr packed_color FOREST_GREEN           = 0X228B22FF;
constexpr packed_color FUCHSIA                = 0xFF00FFFF;
constexpr packed_color GAINSBORO              = 0xDCDCDCFF;
constexpr packed_color GHOST_WHITE            = 0xF8F8FFFF;
constexpr packed_color GOLD                   = 0xFFD700FF;
constexpr packed_color GOLDENROD              = 0xDAA520FF;
constexpr packed_color GRAY                   = 0X808080FF;
constexpr packed_color GREEN                  = 0X008000FF;
constexpr packed_color GREEN_YELLOW           = 0xADFF2FFF;
constexpr packed_color HONEY_DEW              = 0xF0FFF0FF;
constexpr packed_color HOT_PINK               = 0xFF69B4FF;
constexpr packed_color INDIAN_RED             = 0xCD5C5CFF;
constexpr packed_color INDIGO                 = 0X4B0082FF;
constexpr packed_color IVORY                  = 0xFFFFF0FF;
constexpr packed_color KHAKI                  = 0xF0E68CFF;
constexpr packed_color LAVENDER               = 0xE6E6FAFF;
constexpr packed_color LAVENDER_BLUSH         = 0xFFF0F5FF;
constexpr packed_color LAWN_GREEN             = 0X7CFC00FF;
constexpr packed_color LEMON_CHIFFON          = 0xFFFACDFF;
constexpr packed_color LIGHT_BLUE             = 0xADD8E6FF;
constexpr packed_color LIGHT_CORAL            = 0xF08080FF;
constexpr packed_color LIGHT_CYAN             = 0xE0FFFFFF;
constexpr packed_color LIGHT_GOLDENROD_YELLOW = 0xFAFAD2FF;
constexpr packed_color LIGHT_GRAY             = 0xD3D3D3FF;
constexpr packed_color LIGHT_GREEN            = 0X90EE90FF;
constexpr packed_color LIGHT_PINK             = 0xFFB6C1FF;
constexpr packed_color LIGHT_SALMON           = 0xFFA07AFF;
constexpr packed_color LIGHT_SEA_GREEN        = 0X20B2AAFF;
constexpr packed_color LIGHT_SKY_BLUE         = 0X87CEFAFF;
constexpr packed_color LIGHT_SLATE_GRAY       = 0X778899FF;
constexpr packed_color LIGHT_STEEL_BLUE       = 0xB0C4DEFF;
constexpr packed_color LIGHT_YELLOW           = 0xFFFFE0FF;
constexpr packed_color LIME                   = 0X00FF00FF;
constexpr packed_color LIME_GREEN             = 0X32CD32FF;
constexpr packed_color LINEN                  = 0xFAF0E6FF;
constexpr packed_color MAGENTA                = 0xFF00FFFF;
constexpr packed_color MAROON                 = 0X800000FF;
constexpr packed_color MEDIUM_AQUAMARINE      = 0X66CDAAFF;
constexpr packed_color MEDIUM_BLUE            = 0X0000CDFF;
constexpr packed_color MEDIUM_ORCHID          = 0xBA55D3FF;
constexpr packed_color MEDIUM_PURPLE          = 0X9370DBFF;
constexpr packed_color MEDIUM_SEA_GREEN       = 0X3CB371FF;
constexpr packed_color MEDIUM_SLATE_BLUE      = 0X7B68EEFF;
constexpr packed_color MEDIUM_SPRING_GREEN    = 0X00FA9AFF;
constexpr packed_color MEDIUM_TURQUOISE       = 0X48D1CCFF;
constexpr packed_color MEDIUM_VIOLET_RED      = 0xC71585FF;
constexpr packed_color MIDNIGHT_BLUE          = 0X191970FF;
constexpr packed_color MINT_CREAM             = 0xF5FFFAFF;
constexpr packed_color MISTY_ROSE             = 0xFFE4E1FF;
constexpr packed_color MOCCASIN               = 0xFFE4B5FF;
constexpr packed_color NAVAJO_WHITE           = 0xFFDEADFF;
constexpr packed_color NAVY                   = 0X000080FF;
constexpr packed_color OLD_LACE               = 0xFDF5E6FF;
constexpr packed_color OLIVE                  = 0X808000FF;
constexpr packed_color OLIVE_DRAB             = 0X6B8E23FF;
constexpr packed_color ORANGE                 = 0xFFA500FF;
constexpr packed_color ORANGE_RED             = 0xFF4500FF;
constexpr packed_color ORCHID                 = 0xDA70D6FF;
constexpr packed_color PALE_GOLDENROD         = 0xEEE8AAFF;
constexpr packed_color PALE_GREEN             = 0X98FB98FF;
constexpr packed_color PALE_TURQUOISE         = 0xAFEEEEFF;
constexpr packed_color PALE_VIOLET_RED        = 0xDB7093FF;
constexpr packed_color PAPAYA_WHIP            = 0xFFEFD5FF;
constexpr packed_color PEACH_PUFF             = 0xFFDAB9FF;
constexpr packed_color PERU                   = 0xCD853FFF;
constexpr packed_color PINK                   = 0xFFC0CBFF;
constexpr packed_color PLUM                   = 0xDDA0DDFF;
constexpr packed_color POWDER_BLUE            = 0xB0E0E6FF;
constexpr packed_color PURPLE                 = 0X800080FF;
constexpr packed_color REBECCA_PURPLE         = 0X663399FF;
constexpr packed_color RED                    = 0xFF0000FF;
constexpr packed_color ROSY_BROWN             = 0xBC8F8FFF;
constexpr packed_color ROYAL_BLUE             = 0X4169E1FF;
constexpr packed_color SADDLE_BROWN           = 0X8B4513FF;
constexpr packed_color SALMON                 = 0xFA8072FF;
constexpr packed_color SANDY_BROWN            = 0xF4A460FF;
constexpr packed_color SEA_GREEN              = 0X2E8B57FF;
constexpr packed_color SEA_SHELL              = 0xFFF5EEFF;
constexpr packed_color SIENNA                 = 0xA0522DFF;
constexpr packed_color SILVER                 = 0xC0C0C0FF;
constexpr packed_color SKY_BLUE               = 0X87CEEBFF;
constexpr packed_color SLATE_BLUE             = 0X6A5ACDFF;
constexpr packed_color SLATE_GRAY             = 0X708090FF;
constexpr packed_color SNOW                   = 0xFFFAFAFF;
constexpr packed_color SPRING_GREEN           = 0X00FF7FFF;
constexpr packed_color STEEL_BLUE             = 0X4682B4FF;
constexpr packed_color TAN                    = 0xD2B48CFF;
constexpr packed_color TEAL                   = 0X008080FF;
constexpr packed_color THISTLE                = 0xD8BFD8FF;
constexpr packed_color TOMATO                 = 0xFF6347FF;
constexpr packed_color TURQUOISE              = 0X40E0D0FF;
constexpr packed_color VIOLET                 = 0xEE82EEFF;
constexpr packed_color WHEAT                  = 0xF5DEB3FF;
constexpr packed_color WHITE                  = 0xFFFFFFFF;
constexpr packed_color WHITE_SMOKE            = 0xF5F5F5FF;
constexpr packed_color YELLOW                 = 0xFFFF00FF;
constexpr packed_color YELLOW_GREEN           = 0X9ACD32FF;
//! @}


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
    explicit color(packed_color irgba);

    //! @brief Compares colors.
    bool operator==(color const&) const;

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

    //! @brief Prints the content on a stream.
    template <typename O>
    void print(O& o) const {
        o << "rgba(" << 100*rgba[0] << "%," << 100*rgba[1] << "%," << 100*rgba[2] << "%," << 100*rgba[3] << "%)";
    }

    //! @brief Builds a color from its HSVA representation (h maxes to 360, the rest is normalised).
    static color hsva(double h, double s, double v, double a = 1);

    //! @brief The float RGBA components of the color.
    float rgba[4];
};


//! @brief Color addition (for blending).
color operator+(color const&, color const&);

//! @brief Color multiplication (for blending).
color operator*(double, color);


//! @brief Printing colors.
template <typename O>
O& operator<<(O& o, color const& c) {
    c.print(o);
    return o;
}


//! @brief Conversion to string.
std::string to_string(color const&);


}

#endif // FCPP_DATA_COLOR_H_
