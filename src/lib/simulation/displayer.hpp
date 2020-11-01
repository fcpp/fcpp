// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file displayer.hpp
 * @brief Implementation of the `displayer` component representing the simulation status graphically.
 */

#ifndef FCPP_SIMULATION_DISPLAYER_H_
#define FCPP_SIMULATION_DISPLAYER_H_

#include <cassert>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

#include "lib/component/base.hpp"
#include "lib/data/vec.hpp"
#include "lib/graphics/camera.h"
#include "lib/graphics/shader.h"



/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Supported shapes for representing nodes.
enum class shape { cube, sphere };


//! @brief Color type as a packed integer, for usage in template parameters.
using packed_color = uint32_t;


//! @brief Colors for representing nodes.
struct color {
    //! @brief Default color (black).
    color() : rgba{0,0,0,1} {}

    //! @brief Color constructor from float RGBA values.
    color(float r, float g, float b, float a = 1) : rgba{r,g,b,a} {}

    //! @brief Color constructor from integral RGBA values.
    color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : rgba{(float)r/255,(float)g/255,(float)b/255,(float)a/255} {}

    //! @brief Color constructor from a packed integral RGBA value.
    color(packed_color irgba) : color((irgba>>24)&255, (irgba>>16)&255, (irgba>>8)&255, irgba&255) {}

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
    static color hsva(double h, double s, double v, double a = 1) {
        h -= 360 * floor(h/360);
        double c = s*v;
        double x = c*(1-abs(fmod(h/60.0, 2)-1));
        double m = v-c;
        double r,g,b;
        if (h >= 0 and h < 60)
            r = c, g = x, b = 0;
        else if (h >= 60 and h < 120)
            r = x, g = c, b = 0;
        else if(h >= 120 and h < 180)
            r = 0, g = c, b = x;
        else if(h >= 180 and h < 240)
            r = 0, g = x, b = c;
        else if(h >= 240 and h < 300)
            r = x, g = 0, b = c;
        else
            r = c, g = 0, b = x;
        return color((r+m)*255, (g+m)*255, (b+m)*255);
    }

    //! @brief The float RGBA components of the color.
    float rgba[4];
}

//! @brief Builds a packed color from its RGB representation.
constexpr packed_color packed_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return ((uint32_t)r << 24) + ((uint32_t)g << 16) + ((uint32_t)b << 8) + a;
}

//! @brief Builds a packed color from its HSVA representation (h maxes to 360, the rest to 100).
constexpr packed_color packed_hsva(int h, int s, int v, int a = 100) {
    h %= 360;
    int c = s*v;
    int x = c*(60 - abs(h%120 - 60))/60;
    int m = v*100-c;
    int r,g,b;
    if (h >= 0 and h < 60)
        r = c, g = x, b = 0;
    else if (h >= 60 and h < 120)
        r = x, g = c, b = 0;
    else if(h >= 120 and h < 180)
        r = 0, g = c, b = x;
    else if(h >= 180 and h < 240)
        r = 0, g = x, b = c;
    else if(h >= 240 and h < 300)
        r = x, g = 0, b = c;
    else
        r = c, g = 0, b = x;
    return packed_rgba((r+m)*255/10000, (g+m)*255/10000, (b+m)*255/10000);
}


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a storage tag regulating the shape of nodes.
    template <typename T>
    struct shape_tag {};

    //! @brief Declaration tag associating to the base shape of nodes.
    template <size_t n>
    struct shape_val {};

    //! @brief Declaration tag associating to a storage tag regulating the size of nodes.
    template <typename T>
    struct size_tag {};

    //! @brief Declaration tag associating to the base size of nodes.
    template <size_t num, size_t den = 1>
    struct size_val {};

    //! @brief Declaration tag associating to storage tags regulating the colors of nodes.
    template <typename... Ts>
    struct color_tag {};

    //! @brief Declaration tag associating to the base colors of nodes.
    template <size_t... cs>
    struct color_val {};

    //! @brief Declaration flag associating to whether parallelism is enabled.
    template <bool b>
    struct parallel;

    //! @brief Net initialisation tag associating to a factor to be applied to real time.
    struct realtime_factor;

    //! @brief Net initialisation tag associating to the refresh rate.
    struct refresh_rate {};
}


/**
 * @brief Component representing the simulation status graphically.
 *
 * Requires a \ref identifier , \ref positioner and \ref storage parent component.
 * A \ref displayer component cannot be a parent of a \ref timer otherwise refresh rate may be warped.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::shape_tag defines a storage tag regulating the shape of nodes (defaults to none).
 * - \ref tags::shape_val defines the base shape of nodes (defaults to \ref color::sphere).
 * - \ref tags::size_tag defines a storage tag regulating the size of nodes (defaults to none).
 * - \ref tags::size_val defines the base size of nodes (defaults to 1).
 * - \ref tags::color_tag defines storage tags regulating the colors of nodes (defaults to none).
 * - \ref tags::color_val defines the base colors of nodes (defaults to none).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::realtime_factor associates to a `double` factor to be applied to real time (defaults to \ref FCPP_REALTIME).
 * - \ref tags::refresh_rate associates to the refresh rate (defaults to \ref FCPP_REFRESH_RATE).
 *
 * If no color tags or color values are specified, the color defaults to black.
 */
template <class... Ts>
struct displayer {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Storage tag regulating the shape of nodes.
    using shape_tag = common::option_type<tags::shape_tag, void, Ts...>;

    //! @brief Base shape of nodes (defaults to sphere).
    constexpr static shape shape_val = static_cast<shape>(common::option_num<tags::shape_val, static_cast<size_t>(shape::sphere), Ts...>);

    //! @brief Storage tag regulating the size of nodes.
    using size_tag = common::option_type<tags::size_tag, void, Ts...>;

    //! @brief Base size of nodes (defaults to 1).
    constexpr static double size_val = common::option_float<tags::size_val, 1, 1, Ts...>;

    //! @brief Storage tags regulating the colors of nodes.
    using color_tag = common::option_types<tags::color_tag, Ts...>;

    //! @brief Base colors of nodes (defaults to black).
    using color_val = common::option_nums<tags::color_val, Ts...>;

    /**
     * @brief The actual component.
     *
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template <typename F, typename P>
    struct component : public P {
        DECLARE_COMPONENT(displayer);
        REQUIRE_COMPONENT(displayer,identifier);
        REQUIRE_COMPONENT(displayer,positioner);
        REQUIRE_COMPONENT(displayer,storage);
        CHECK_COMPONENT(timer);

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_nbr_uids(), m_prev_nbr_uids(), m_pos_time(0), m_refresh(0) {}

            /**
             * @brief Returns next event to schedule for the node component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_refresh, P::node::next());
            }

            //! @brief Updates the internal status of node component.
            void update() {
                if (m_refresh < P::node::next()) {
                    PROFILE_COUNT("displayer");
                    glm::vec3 p = get_cached_position(m_refresh);
                    std::vector<glm::vec3> np;
                    // update shape and size
                    shape s = common::get_or<shape_tag>(P::node::storage_tuple(), shape(shape_val));
                    double d = common::get_or<size_tag>(P::node::storage_tuple(), double(size_val));
                    // update color list
                    std::vector<color> c;
                    color_val_push(c, color_val{});
                    color_tag_push(c, color_tag{});
                    if (c.empty()) c.push_back(rgb(0, 0, 0)); // black if nothing else
                    {
                        common::unlock_guard<parallel> ul(P::node::mutex);
                        for (device_t d : m_prev_nbr_uids) {
                            common::unique_lock<parallel> l;
                            np.push_back(P::node::net.node_at(d, l).get_cached_position(m_refresh));
                        }
                        /**
                         * Do not touch the code above.
                         *
                         * Use this function to update node positions
                         * (possibly more often than shape/size/colors).
                         * You find the current position in p, and
                         * neighbours' positions in np (for displaying links).
                         *
                         * Update the openGL representation somehow below.
                         */
                        P::node::net.m_shaderProgram.use();

                        common::details::ignore(p,np); // remove this line
                    }
                    m_refresh += P::node::net.refresh_step();
                } else P::node::update();
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                PROFILE_COUNT("displayer");
                // skip if the next update is before the next refresh
                if (P::node::next() < m_refresh) return;
                // update neighbours list
                std::sort(m_nbr_uids.begin(), m_nbr_uids.end());
                m_nbr_uids.erase(std::unique(m_nbr_uids.begin(), m_nbr_uids.end()), m_nbr_uids.end());
                m_prev_nbr_uids = std::move(m_nbr_uids);
                m_nbr_uids.clear();
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                m_nbr_uids.push_back(d);
            }

          private: // implementation details
            //! @brief Conversion to 3D vector (trivial case).
            glm::vec3 to_vec3(vec<3> p) const {
                return { p[0], p[1], p[2] };
            }

            //! @brief Conversion to 3D vector (non-trivial case).
            glm::vec3 to_vec3(vec<2> p) const {
                return { p[0], p[1], 0 };
            }

            //! @brief Gets the current position using caching.
            glm::vec3 get_cached_position(times_t t) {
                if (t > m_pos_time) {
                    m_position = to_vec3(P::node::position(t));
                    m_pos_time = t;
                }
                return m_position;
            }

            //! @brief Pushes colors in an index sequence into a vector (base case).
            void color_val_push(std::vector<color>&, common::index_sequence<>) const {}

            //! @brief Pushes colors in an index sequence into a vector (inductive case).
            template <size_t i, size_t... is>
            void color_val_push(std::vector<color>& c, common::index_sequence<i, is...>) const {
                c.push_back(i);
                color_val_push(c, common::index_sequence<is...>{});
            }

            //! @brief Pushes colors from storage tags into a vector (base case).
            void color_tag_push(std::vector<color>&, common::type_sequence<>) const {}

            //! @brief Pushes colors from storage tags into a vector (inductive case).
            template <typename S, typename... Ss>
            void color_tag_push(std::vector<color>& c, common::type_sequence<S, Ss...>) const {
                c.push_back(P::node::storage(S{}));
                color_tag_push(c, common::type_sequence<Ss...>{});
            }

            //! @brief The current position of the device.
            glm::vec3 m_position;

            //! @brief The uids of incoming messages.
            std::vector<device_t> m_nbr_uids;

            //! @brief The uids of incoming messages during the previous round.
            std::vector<device_t> m_prev_nbr_uids;

            //! @brief The cached position time.
            times_t m_pos_time;

            //! @brief The next refresh time.
            times_t m_refresh;
        };

        //! @brief The global part of the component.
        class net : public P::net {
        public: // visible by node objects and the main program
          //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S, T>& t) :
                P::net{ t },
                m_refresh{ 0 },
                LIGHT_DEFAULT_POS{ glm::vec3{0.0f, 0.0f, 0.0f} },
#ifdef _WIN32
                VERTEX_PATH{ ".\\shaders\\vertex.glsl" },
                FRAGMENT_PATH{ ".\\shaders\\fragment.glsl" },
                VERTEX_COLOR_PATH{ ".\\shaders\\vertex_col.glsl" },
                FRAGMENT_COLOR_PATH{ ".\\shaders\\fragment_col.glsl" },
#else
                VERTEX_PATH{ "./shaders/vertex.glsl" },
                FRAGMENT_PATH{ "./shaders/fragment.glsl" },
                VERTEX_COLOR_PATH{ "./shaders/vertex_col.glsl" },
                FRAGMENT_COLOR_PATH{ "./shaders/fragment_col.glsl" },
#endif
                SCR_DEFAULT_WIDTH{ 800 },
                SCR_DEFAULT_HEIGHT{ 600 },
                SCR_DEFAULT_ORTHO{ 64 },
                m_currentWidth{ SCR_DEFAULT_WIDTH },
                m_currentHeight{ SCR_DEFAULT_HEIGHT },
                m_orthoSize{ SCR_DEFAULT_ORTHO },
                m_camera{ glm::vec3{ 0.0f, 0.0f, 3.0f } },
                m_mouseLastX{ (float)(SCR_DEFAULT_WIDTH / 2) },
                m_mouseLastY{ (float)(SCR_DEFAULT_HEIGHT / 2) },
                m_mouseFirst{ 1 },
                m_deltaTime{ 0.0f },
                m_lastFrame{ 0.0f },
                VERTEX_CUBE{
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
                },
                VERTEX_ORTHO{
                    // positions              // colors
                    -0.01f,  0.01f,  0.01f,   0.0f, 0.5f, 0.0f,
                     0.01f,  0.01f,  0.01f,   0.0f, 0.5f, 0.0f,
                     0.01f,  0.01f, -0.01f,   0.0f, 0.5f, 0.0f,
                    -0.01f,  0.01f, -0.01f,   0.0f, 0.5f, 0.0f,
                    -0.01f,  0.60f,  0.01f,   0.0f, 1.0f, 0.0f,
                     0.01f,  0.60f,  0.01f,   0.0f, 1.0f, 0.0f,
                     0.01f,  0.60f, -0.01f,   0.0f, 1.0f, 0.0f,
                    -0.01f,  0.60f, -0.01f,   0.0f, 1.0f, 0.0f,

                     0.01f,  0.01f,  0.01f,   0.5f, 0.0f, 0.0f,
                     0.01f,  0.01f, -0.01f,   0.5f, 0.0f, 0.0f,
                     0.01f, -0.01f, -0.01f,   0.5f, 0.0f, 0.0f,
                     0.01f, -0.01f,  0.01f,   0.5f, 0.0f, 0.0f,
                     0.60f,  0.01f,  0.01f,   1.0f, 0.0f, 0.0f,
                     0.60f,  0.01f, -0.01f,   1.0f, 0.0f, 0.0f,
                     0.60f, -0.01f, -0.01f,   1.0f, 0.0f, 0.0f,
                     0.60f, -0.01f,  0.01f,   1.0f, 0.0f, 0.0f,

                    -0.01f,  0.01f,  0.01f,   0.0f, 0.0f, 0.5f,
                     0.01f,  0.01f,  0.01f,   0.0f, 0.0f, 0.5f,
                     0.01f, -0.01f,  0.01f,   0.0f, 0.0f, 0.5f,
                    -0.01f, -0.01f,  0.01f,   0.0f, 0.0f, 0.5f,
                    -0.01f,  0.01f,  0.60f,   0.0f, 0.0f, 1.0f,
                     0.01f,  0.01f,  0.60f,   0.0f, 0.0f, 1.0f,
                     0.01f, -0.01f,  0.60f,   0.0f, 0.0f, 1.0f,
                    -0.01f, -0.01f,  0.60f,   0.0f, 0.0f, 1.0f
                },
                INDEX_ORTHO{
                    0, 1, 2,
                    2, 3, 0,
                    4, 5, 6,
                    6, 7, 4,
                    0, 1, 4,
                    4, 7, 0,
                    1, 2, 5,
                    5, 4, 1,
                    2, 3, 6,
                    6, 5, 2,
                    3, 0, 7,
                    7, 6, 3,

                    8, 9, 10,
                    10, 11, 8,
                    12, 13, 14,
                    14, 15, 12,
                    8, 12, 15,
                    15, 11, 8,
                    8, 12, 13,
                    13, 8, 9,
                    9, 13, 10,
                    10, 13, 14,
                    14, 15, 10,
                    10, 11, 15,

                    16, 17, 18,
                    18, 19, 16,
                    20, 21, 22,
                    22, 23, 20,
                    20, 16, 17,
                    17, 20, 21,
                    21, 17, 18,
                    18, 21, 22,
                    22, 18, 19,
                    19, 22, 23,
                    23, 19, 16,
                    16, 23, 20
                } {
                // Set simulation refresh rate
                m_step = common::get_or<tags::refresh_rate>(t, FCPP_REFRESH_RATE) * common::get_or<tags::realtime_factor>(t, FCPP_REALTIME);

                // Initialize GLFW
                glfwInit();

                // Set context options
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
                // Create window (and its context)
                m_window = glfwCreateWindow(SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT, "fcppGL", NULL, NULL);

                if (m_window == NULL) {
                    glfwTerminate();
                    throw std::runtime_error("Failed to create GLFW window.\n");
                }

                // Set newly created window's context as current
                glfwMakeContextCurrent(m_window);

                // Initialize GLAD
                if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
                    throw std::runtime_error("Failed to initialize GLAD.\n");

                // Associates this (the net instance) to m_window
                glfwSetWindowUserPointer(m_window, this);

                // Set viewport and its callbacks
                glViewport(0, 0, SCR_DEFAULT_WIDTH, SCR_DEFAULT_HEIGHT);
                glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
                    net& n = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    n.framebufferSizeCallback(window, width, height);
                });

                // Enable cursor capture and callbacks
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
                    net& n = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    n.mouseCallback(window, xpos, ypos);
                });
                glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
                    net& n = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    n.scrollCallback(window, xoffset, yoffset);
                });

                // Generate actual shader programs
                m_shaderProgram = Shader{ VERTEX_PATH.c_str(), FRAGMENT_PATH.c_str() };
                m_shaderProgramCol = Shader{ VERTEX_COLOR_PATH.c_str(), FRAGMENT_COLOR_PATH.c_str() };

                // Generate VAOs, VBOs and EBOs
                glGenVertexArrays(2, VAO);
                glGenBuffers(2, VBO);
                glGenBuffers(1, EBO);

                // Store ortho data
                glBindVertexArray(VAO[0]);
                glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_ORTHO), VERTEX_ORTHO, GL_STATIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]); // VAO[0] stores EBO[0] here; do NOT unbind EBO[0] until VAO[0] is unbound
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDEX_ORTHO), INDEX_ORTHO, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                // Store cube data
                glBindVertexArray(VAO[1]);
                glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX_CUBE), VERTEX_CUBE, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                // Enabling depth test and blending
                glEnable(GL_DEPTH_TEST);

                // Uncomment this call to draw in wireframe polygons
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_refresh, P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                if (m_refresh < P::net::next()) {
                    PROFILE_COUNT("displayer");
                    times_t t = get_warped(has_timer<P>{}, *this, m_refresh);
                    /**
                     * Do whatever global management you need to do here
                     * (drawing the scene, interpreting the user input).
                     * You may want to display the current simulation time t.
                     */

                    // Deltatime
                    float currentFrame{ (float)glfwGetTime() };
                    m_deltaTime = currentFrame - m_lastFrame;
                    m_lastFrame = currentFrame;

                    // Input
                    processKeyboardInput(m_window);

                    // Clear frame
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    // Create matrices (used several times)
                    glm::mat4 projection{ glm::perspective(glm::radians(m_camera.getFov()), (float)m_currentWidth / (float)m_currentHeight, 0.1f, 100.0f) };
                    glm::mat4 view{ m_camera.getViewMatrix() };
                    glm::mat4 model{ 1.0f };
                    glm::mat3 normal;

                    // Draw ortho
                    glClear(GL_DEPTH_BUFFER_BIT); // Clean depth buffer, in order to draw on top of 3D objects
                    m_shaderProgramCol.use();
                    glBindVertexArray(VAO[0]);
                    projection = glm::ortho(0.0f, (float)m_currentWidth, 0.0f, (float)m_currentHeight, -1.0f, (float)m_orthoSize);
                    m_shaderProgramCol.setMat4("u_projection", projection);
                    view = glm::mat4{ 1.0f };
                    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -(float)m_orthoSize / 2.0f));
                    m_shaderProgramCol.setMat4("u_view", view);
                    model = glm::mat4{ 1.0f };
                    model = glm::translate(model, glm::vec3((float)m_currentWidth - ((float)m_orthoSize * 3.0f / 4.0f), (float)m_orthoSize * 3.0f / 4.0f, 0.0f));
                    model = glm::rotate(model, glm::radians(-m_camera.getPitch()), glm::vec3(1.0f, 0.0f, 0.0f));
                    model = glm::rotate(model, glm::radians(m_camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3((float)m_orthoSize));
                    m_shaderProgramCol.setMat4("u_model", model);
                    glDrawElements(GL_TRIANGLES, sizeof(INDEX_ORTHO) / sizeof(int), GL_UNSIGNED_INT, 0);

                    // Check and call events, swap double buffers
                    glfwPollEvents();
                    glfwSwapBuffers(m_window);

                    common::details::ignore(t); // remove this line
                    m_refresh += m_step;
                } else P::net::update();
            }

            //! @brief Returns the refresh step in subjective node time.
            times_t refresh_step() const {
                return get_warped(has_timer<P>{}, *this, m_step);
            }

          private: // implementation details
            //! @brief Converts to warped time.
            template <typename N>
            inline times_t get_warped(std::true_type, N const& n, times_t t) const {
                return n.warped_time(t);
            }

            //! @brief Converts to warped time.
            template <typename N>
            inline times_t get_warped(std::false_type, N const&, times_t t) const {
                return t;
            }

            //! @brief Keyboard input updater function (not a callback).
            void processKeyboardInput(GLFWwindow* window) {
                //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                //    glfwSetWindowShouldClose(window, true);
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    m_camera.processKeyboard(FORWARD, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    m_camera.processKeyboard(BACKWARD, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    m_camera.processKeyboard(LEFT, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    m_camera.processKeyboard(RIGHT, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                    m_camera.processKeyboard(FLY_UP, m_deltaTime);
                if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                    m_camera.processKeyboard(FLY_DOWN, m_deltaTime);
            }

            //! @brief Mouse input callback function.
            void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
                if (m_mouseFirst) {
                    m_mouseLastX = (float)xpos;
                    m_mouseLastY = (float)ypos;
                    m_mouseFirst = false;
                }

                float xoffset{ (float)(xpos - m_mouseLastX) };
                float yoffset{ (float)(m_mouseLastY - ypos) }; // reversed since y-coordinates range from bottom to top
                m_mouseLastX = (float)xpos;
                m_mouseLastY = (float)ypos;

                m_camera.processMouseMovement(xoffset, yoffset);
            }

            //! @brief Mouse scroll callback function.
            void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
                m_camera.processMouseScroll((float)yoffset);
            }

            //! @brief Window resize callback function.
            void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
                glViewport(0, 0, width, height);
                m_currentWidth = width;
                m_currentHeight = height;
            }

            //! @brief The next refresh time.
            times_t m_refresh;

            //! @brief The step between refresh times.
            times_t m_step;

            //! @brief Window object for GLFW; it stores OpenGL context information.
            GLFWwindow* m_window;

            //! @brief Main shader program.
            Shader m_shaderProgram;

            //! @brief Additional shader program for reading vertex buffers with color info; used for orthogonal axis.
            Shader m_shaderProgramCol;

            //! @brief Vertex Array Object(s).
            unsigned int VAO[2];

            //! @brief Vertex Buffer Object(s).
            unsigned int VBO[2];

            //! @brief Element Buffer Object(s).
            unsigned int EBO[1];

            //! @brief Default light position.
            const glm::vec3 LIGHT_DEFAULT_POS;

            //! @brief Default path to vertex shader.
            const std::string VERTEX_PATH;

            //! @brief Default path to fragment shader.
            const std::string FRAGMENT_PATH;

            //! @brief Default path to vertex_col shader.
            const std::string VERTEX_COLOR_PATH;

            //! @brief Default path to fragment_col shader.
            const std::string FRAGMENT_COLOR_PATH;

            //! @brief Default width of the window.
            const unsigned int SCR_DEFAULT_WIDTH;

            //! @brief Default height of the window.
            const unsigned int SCR_DEFAULT_HEIGHT;

            //! @brief Default size of orthogonal axis.
            const unsigned int SCR_DEFAULT_ORTHO;

            //! @brief Current width of the window.
            unsigned int m_currentWidth;

            //! @brief Current height of the window.
            unsigned int m_currentHeight;

            //! @brief Current size of orthogonal axis.
            unsigned int m_orthoSize;

            //! @brief Camera object of the scene
            Camera m_camera;

            //! @brief Last mouse X position.
            float m_mouseLastX;

            //! @brief Last mouse Y position.
            float m_mouseLastY; 

            //!@brief It checks if it's the first mouse's input capture.
            bool m_mouseFirst;

            //!@brief Time between current frame and last frame.
            float m_deltaTime;

            //!@brief Ttime of last frame.
            float m_lastFrame;

            //! @brief Cube's vertex data.
            const float VERTEX_CUBE[216];

            //! @brief Orthogonal axis' vertex data.
            const float VERTEX_ORTHO[144];

            //! @brief Orthogonal axis' index data.
            const unsigned int INDEX_ORTHO[108];
        };
    };
};


}


}

#endif // FCPP_SIMULATION_DISPLAYER_H_
