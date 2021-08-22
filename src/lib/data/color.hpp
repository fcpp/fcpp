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

//! @brief Alice Blue HTML color.
constexpr packed_color ALICE_BLUE             = 0xF0F8FFFF;
//! @brief Antique White HTML color.
constexpr packed_color ANTIQUE_WHITE          = 0xFAEBD7FF;
//! @brief Aqua HTML color.
constexpr packed_color AQUA                   = 0X00FFFFFF;
//! @brief Aquamarine HTML color.
constexpr packed_color AQUAMARINE             = 0X7FFFD4FF;
//! @brief Azure HTML color.
constexpr packed_color AZURE                  = 0xF0FFFFFF;
//! @brief Beige HTML color.
constexpr packed_color BEIGE                  = 0xF5F5DCFF;
//! @brief Bisque HTML color.
constexpr packed_color BISQUE                 = 0xFFE4C4FF;
//! @brief Black HTML color.
constexpr packed_color BLACK                  = 0X000000FF;
//! @brief Blanched Almond HTML color.
constexpr packed_color BLANCHED_ALMOND        = 0xFFEBCDFF;
//! @brief Blue HTML color.
constexpr packed_color BLUE                   = 0X0000FFFF;
//! @brief Blue Violet HTML color.
constexpr packed_color BLUE_VIOLET            = 0X8A2BE2FF;
//! @brief Brown HTML color.
constexpr packed_color BROWN                  = 0xA52A2AFF;
//! @brief Burly Wood HTML color.
constexpr packed_color BURLY_WOOD             = 0xDEB887FF;
//! @brief Cadet Blue HTML color.
constexpr packed_color CADET_BLUE             = 0X5F9EA0FF;
//! @brief Chartreuse HTML color.
constexpr packed_color CHARTREUSE             = 0X7FFF00FF;
//! @brief Chocolate HTML color.
constexpr packed_color CHOCOLATE              = 0xD2691EFF;
//! @brief Coral HTML color.
constexpr packed_color CORAL                  = 0xFF7F50FF;
//! @brief Cornflower Blue HTML color.
constexpr packed_color CORNFLOWER_BLUE        = 0X6495EDFF;
//! @brief Cornsilk HTML color.
constexpr packed_color CORNSILK               = 0xFFF8DCFF;
//! @brief Crimson HTML color.
constexpr packed_color CRIMSON                = 0xDC143CFF;
//! @brief Cyan HTML color.
constexpr packed_color CYAN                   = 0X00FFFFFF;
//! @brief Dark Blue HTML color.
constexpr packed_color DARK_BLUE              = 0X00008BFF;
//! @brief Dark Cyan HTML color.
constexpr packed_color DARK_CYAN              = 0X008B8BFF;
//! @brief Dark Goldenrod HTML color.
constexpr packed_color DARK_GOLDENROD         = 0xB8860BFF;
//! @brief Dark Gray HTML color.
constexpr packed_color DARK_GRAY              = 0xA9A9A9FF;
//! @brief Dark Green HTML color.
constexpr packed_color DARK_GREEN             = 0X006400FF;
//! @brief Dark Khaki HTML color.
constexpr packed_color DARK_KHAKI             = 0xBDB76BFF;
//! @brief Dark Magenta HTML color.
constexpr packed_color DARK_MAGENTA           = 0X8B008BFF;
//! @brief Dark Olive Green HTML color.
constexpr packed_color DARK_OLIVE_GREEN       = 0X556B2FFF;
//! @brief Dark Orange HTML color.
constexpr packed_color DARK_ORANGE            = 0xFF8C00FF;
//! @brief Dark Orchid HTML color.
constexpr packed_color DARK_ORCHID            = 0X9932CCFF;
//! @brief Dark Red HTML color.
constexpr packed_color DARK_RED               = 0X8B0000FF;
//! @brief Dark Salmon HTML color.
constexpr packed_color DARK_SALMON            = 0xE9967AFF;
//! @brief Dark Sea Green HTML color.
constexpr packed_color DARK_SEA_GREEN         = 0X8FBC8BFF;
//! @brief Dark Slate Blue HTML color.
constexpr packed_color DARK_SLATE_BLUE        = 0X483D8BFF;
//! @brief Dark Slate Gray HTML color.
constexpr packed_color DARK_SLATE_GRAY        = 0X2F4F4FFF;
//! @brief Dark Turquoise HTML color.
constexpr packed_color DARK_TURQUOISE         = 0X00CED1FF;
//! @brief Dark Violet HTML color.
constexpr packed_color DARK_VIOLET            = 0X9400D3FF;
//! @brief Deep Pink HTML color.
constexpr packed_color DEEP_PINK              = 0xFF1493FF;
//! @brief Deep Sky Blue HTML color.
constexpr packed_color DEEP_SKY_BLUE          = 0X00BFFFFF;
//! @brief Dim Gray HTML color.
constexpr packed_color DIM_GRAY               = 0X696969FF;
//! @brief Dodger Blue HTML color.
constexpr packed_color DODGER_BLUE            = 0X1E90FFFF;
//! @brief Fire Brick HTML color.
constexpr packed_color FIRE_BRICK             = 0xB22222FF;
//! @brief Floral White HTML color.
constexpr packed_color FLORAL_WHITE           = 0xFFFAF0FF;
//! @brief Forest Green HTML color.
constexpr packed_color FOREST_GREEN           = 0X228B22FF;
//! @brief Fuchsia HTML color.
constexpr packed_color FUCHSIA                = 0xFF00FFFF;
//! @brief Gainsboro HTML color.
constexpr packed_color GAINSBORO              = 0xDCDCDCFF;
//! @brief Ghost White HTML color.
constexpr packed_color GHOST_WHITE            = 0xF8F8FFFF;
//! @brief Gold HTML color.
constexpr packed_color GOLD                   = 0xFFD700FF;
//! @brief Goldenrod HTML color.
constexpr packed_color GOLDENROD              = 0xDAA520FF;
//! @brief Gray HTML color.
constexpr packed_color GRAY                   = 0X808080FF;
//! @brief Green HTML color.
constexpr packed_color GREEN                  = 0X008000FF;
//! @brief Green Yellow HTML color.
constexpr packed_color GREEN_YELLOW           = 0xADFF2FFF;
//! @brief Honey Dew HTML color.
constexpr packed_color HONEY_DEW              = 0xF0FFF0FF;
//! @brief Hot Pink HTML color.
constexpr packed_color HOT_PINK               = 0xFF69B4FF;
//! @brief Indian Red HTML color.
constexpr packed_color INDIAN_RED             = 0xCD5C5CFF;
//! @brief Indigo HTML color.
constexpr packed_color INDIGO                 = 0X4B0082FF;
//! @brief Ivory HTML color.
constexpr packed_color IVORY                  = 0xFFFFF0FF;
//! @brief Khaki HTML color.
constexpr packed_color KHAKI                  = 0xF0E68CFF;
//! @brief Lavender HTML color.
constexpr packed_color LAVENDER               = 0xE6E6FAFF;
//! @brief Lavender Blush HTML color.
constexpr packed_color LAVENDER_BLUSH         = 0xFFF0F5FF;
//! @brief Lawn Green HTML color.
constexpr packed_color LAWN_GREEN             = 0X7CFC00FF;
//! @brief Lemon Chiffon HTML color.
constexpr packed_color LEMON_CHIFFON          = 0xFFFACDFF;
//! @brief Light Blue HTML color.
constexpr packed_color LIGHT_BLUE             = 0xADD8E6FF;
//! @brief Light Coral HTML color.
constexpr packed_color LIGHT_CORAL            = 0xF08080FF;
//! @brief Light Cyan HTML color.
constexpr packed_color LIGHT_CYAN             = 0xE0FFFFFF;
//! @brief Light Goldenrod Yellow HTML color.
constexpr packed_color LIGHT_GOLDENROD_YELLOW = 0xFAFAD2FF;
//! @brief Light Gray HTML color.
constexpr packed_color LIGHT_GRAY             = 0xD3D3D3FF;
//! @brief Light Green HTML color.
constexpr packed_color LIGHT_GREEN            = 0X90EE90FF;
//! @brief Light Pink HTML color.
constexpr packed_color LIGHT_PINK             = 0xFFB6C1FF;
//! @brief Light Salmon HTML color.
constexpr packed_color LIGHT_SALMON           = 0xFFA07AFF;
//! @brief Light Sea Green HTML color.
constexpr packed_color LIGHT_SEA_GREEN        = 0X20B2AAFF;
//! @brief Light Sky Blue HTML color.
constexpr packed_color LIGHT_SKY_BLUE         = 0X87CEFAFF;
//! @brief Light Slate Gray HTML color.
constexpr packed_color LIGHT_SLATE_GRAY       = 0X778899FF;
//! @brief Light Steel Blue HTML color.
constexpr packed_color LIGHT_STEEL_BLUE       = 0xB0C4DEFF;
//! @brief Light Yellow HTML color.
constexpr packed_color LIGHT_YELLOW           = 0xFFFFE0FF;
//! @brief Lime HTML color.
constexpr packed_color LIME                   = 0X00FF00FF;
//! @brief Lime Green HTML color.
constexpr packed_color LIME_GREEN             = 0X32CD32FF;
//! @brief Linen HTML color.
constexpr packed_color LINEN                  = 0xFAF0E6FF;
//! @brief Magenta HTML color.
constexpr packed_color MAGENTA                = 0xFF00FFFF;
//! @brief Maroon HTML color.
constexpr packed_color MAROON                 = 0X800000FF;
//! @brief Medium Aquamarine HTML color.
constexpr packed_color MEDIUM_AQUAMARINE      = 0X66CDAAFF;
//! @brief Medium Blue HTML color.
constexpr packed_color MEDIUM_BLUE            = 0X0000CDFF;
//! @brief Medium Orchid HTML color.
constexpr packed_color MEDIUM_ORCHID          = 0xBA55D3FF;
//! @brief Medium Purple HTML color.
constexpr packed_color MEDIUM_PURPLE          = 0X9370DBFF;
//! @brief Medium Sea Green HTML color.
constexpr packed_color MEDIUM_SEA_GREEN       = 0X3CB371FF;
//! @brief Medium Slate Blue HTML color.
constexpr packed_color MEDIUM_SLATE_BLUE      = 0X7B68EEFF;
//! @brief Medium Spring Green HTML color.
constexpr packed_color MEDIUM_SPRING_GREEN    = 0X00FA9AFF;
//! @brief Medium Turquoise HTML color.
constexpr packed_color MEDIUM_TURQUOISE       = 0X48D1CCFF;
//! @brief Medium Violet Red HTML color.
constexpr packed_color MEDIUM_VIOLET_RED      = 0xC71585FF;
//! @brief Midnight Blue HTML color.
constexpr packed_color MIDNIGHT_BLUE          = 0X191970FF;
//! @brief Mint Cream HTML color.
constexpr packed_color MINT_CREAM             = 0xF5FFFAFF;
//! @brief Misty Rose HTML color.
constexpr packed_color MISTY_ROSE             = 0xFFE4E1FF;
//! @brief Moccasin HTML color.
constexpr packed_color MOCCASIN               = 0xFFE4B5FF;
//! @brief Navajo White HTML color.
constexpr packed_color NAVAJO_WHITE           = 0xFFDEADFF;
//! @brief Navy HTML color.
constexpr packed_color NAVY                   = 0X000080FF;
//! @brief Old Lace HTML color.
constexpr packed_color OLD_LACE               = 0xFDF5E6FF;
//! @brief Olive HTML color.
constexpr packed_color OLIVE                  = 0X808000FF;
//! @brief Olive Drab HTML color.
constexpr packed_color OLIVE_DRAB             = 0X6B8E23FF;
//! @brief Orange HTML color.
constexpr packed_color ORANGE                 = 0xFFA500FF;
//! @brief Orange Red HTML color.
constexpr packed_color ORANGE_RED             = 0xFF4500FF;
//! @brief Orchid HTML color.
constexpr packed_color ORCHID                 = 0xDA70D6FF;
//! @brief Pale Goldenrod HTML color.
constexpr packed_color PALE_GOLDENROD         = 0xEEE8AAFF;
//! @brief Pale Green HTML color.
constexpr packed_color PALE_GREEN             = 0X98FB98FF;
//! @brief Pale Turquoise HTML color.
constexpr packed_color PALE_TURQUOISE         = 0xAFEEEEFF;
//! @brief Pale Violet Red HTML color.
constexpr packed_color PALE_VIOLET_RED        = 0xDB7093FF;
//! @brief Papaya Whip HTML color.
constexpr packed_color PAPAYA_WHIP            = 0xFFEFD5FF;
//! @brief Peach Puff HTML color.
constexpr packed_color PEACH_PUFF             = 0xFFDAB9FF;
//! @brief Peru HTML color.
constexpr packed_color PERU                   = 0xCD853FFF;
//! @brief Pink HTML color.
constexpr packed_color PINK                   = 0xFFC0CBFF;
//! @brief Plum HTML color.
constexpr packed_color PLUM                   = 0xDDA0DDFF;
//! @brief Powder Blue HTML color.
constexpr packed_color POWDER_BLUE            = 0xB0E0E6FF;
//! @brief Purple HTML color.
constexpr packed_color PURPLE                 = 0X800080FF;
//! @brief Rebecca Purple HTML color.
constexpr packed_color REBECCA_PURPLE         = 0X663399FF;
//! @brief Red HTML color.
constexpr packed_color RED                    = 0xFF0000FF;
//! @brief Rosy Brown HTML color.
constexpr packed_color ROSY_BROWN             = 0xBC8F8FFF;
//! @brief Royal Blue HTML color.
constexpr packed_color ROYAL_BLUE             = 0X4169E1FF;
//! @brief Saddle Brown HTML color.
constexpr packed_color SADDLE_BROWN           = 0X8B4513FF;
//! @brief Salmon HTML color.
constexpr packed_color SALMON                 = 0xFA8072FF;
//! @brief Sandy Brown HTML color.
constexpr packed_color SANDY_BROWN            = 0xF4A460FF;
//! @brief Sea Green HTML color.
constexpr packed_color SEA_GREEN              = 0X2E8B57FF;
//! @brief Sea Shell HTML color.
constexpr packed_color SEA_SHELL              = 0xFFF5EEFF;
//! @brief Sienna HTML color.
constexpr packed_color SIENNA                 = 0xA0522DFF;
//! @brief Silver HTML color.
constexpr packed_color SILVER                 = 0xC0C0C0FF;
//! @brief Sky Blue HTML color.
constexpr packed_color SKY_BLUE               = 0X87CEEBFF;
//! @brief Slate Blue HTML color.
constexpr packed_color SLATE_BLUE             = 0X6A5ACDFF;
//! @brief Slate Gray HTML color.
constexpr packed_color SLATE_GRAY             = 0X708090FF;
//! @brief Snow HTML color.
constexpr packed_color SNOW                   = 0xFFFAFAFF;
//! @brief Spring Green HTML color.
constexpr packed_color SPRING_GREEN           = 0X00FF7FFF;
//! @brief Steel Blue HTML color.
constexpr packed_color STEEL_BLUE             = 0X4682B4FF;
//! @brief Tan HTML color.
constexpr packed_color TAN                    = 0xD2B48CFF;
//! @brief Teal HTML color.
constexpr packed_color TEAL                   = 0X008080FF;
//! @brief Thistle HTML color.
constexpr packed_color THISTLE                = 0xD8BFD8FF;
//! @brief Tomato HTML color.
constexpr packed_color TOMATO                 = 0xFF6347FF;
//! @brief Turquoise HTML color.
constexpr packed_color TURQUOISE              = 0X40E0D0FF;
//! @brief Violet HTML color.
constexpr packed_color VIOLET                 = 0xEE82EEFF;
//! @brief Wheat HTML color.
constexpr packed_color WHEAT                  = 0xF5DEB3FF;
//! @brief White HTML color.
constexpr packed_color WHITE                  = 0xFFFFFFFF;
//! @brief White Smoke HTML color.
constexpr packed_color WHITE_SMOKE            = 0xF5F5F5FF;
//! @brief Yellow HTML color.
constexpr packed_color YELLOW                 = 0xFFFF00FF;
//! @brief Yellow Green HTML color.
constexpr packed_color YELLOW_GREEN           = 0X9ACD32FF;


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
