// Copyright © 2023 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file displayer.hpp
 * @brief Implementation of the `displayer` component representing the simulation status graphically.
 */

#ifndef FCPP_SIMULATION_DISPLAYER_H_
#define FCPP_SIMULATION_DISPLAYER_H_

#include <cmath>
#include <cstdint>
#include <cstring>

#ifdef FCPP_GUI
#include <algorithm>
#include <chrono>
#include <deque>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "lib/common/algorithm.hpp"
#include "lib/common/ostream.hpp"
#include "lib/component/calculus.hpp"
#include "lib/data/field.hpp"
#include "lib/data/vec.hpp"
#include "lib/graphics/renderer.hpp"
#endif


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

// Namespace for all FCPP components.
namespace component {

// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a storage tag regulating the shape of nodes (defaults to none).
    template <typename T>
    struct shape_tag {};

    //! @brief Declaration tag associating to the base shape of nodes (defaults to cube).
    template <intmax_t n>
    struct shape_val {};

    //! @brief Declaration tag associating to a storage tag regulating the size of nodes (defaults to none).
    template <typename T>
    struct size_tag {};

    //! @brief Declaration tag associating to the base size of nodes (defaults to 1).
    template <intmax_t num, intmax_t den = 1>
    struct size_val {};

    //! @brief Declaration tag associating to storage tags regulating the colors of nodes (defaults to none).
    template <typename... Ts>
    struct color_tag {};

    //! @brief Declaration tag associating to the base colors of nodes (defaults to white).
    template <intmax_t... cs>
    struct color_val {};

    //! @brief Declaration tag associating to storage tags with the text of the node labels (defaults to no text).
    template <typename T>
    struct label_text_tag {};

    //! @brief Declaration tag associating to storage tags regulating the size of node labels (defaults to none).
    template <typename T>
    struct label_size_tag {};

    //! @brief Declaration tag associating to the base size of node labels (defaults to 1).
    template <intmax_t num, intmax_t den = 1>
    struct label_size_val {};

    //! @brief Declaration tag associating to storage tags regulating the color of node labels (defaults to none).
    template <typename T>
    struct label_color_tag {};

    //! @brief Declaration tag associating to the base colors of node labels (defaults to black).
    template <intmax_t c>
    struct label_color_val {};

    //! @brief Declaration tag associating to a storage tag regulating the shape of node shadows (defaults to none).
    template <typename T>
    struct shadow_shape_tag {};

    //! @brief Declaration tag associating to the base shape of node shadows (defaults to the same shape as the node).
    template <intmax_t n>
    struct shadow_shape_val {};

    //! @brief Declaration tag associating to storage tags regulating the color of node shadows (defaults to none).
    template <typename T>
    struct shadow_color_tag {};

    //! @brief Declaration tag associating to the base colors of node shadows (defaults to the same color as the node).
    template <intmax_t c>
    struct shadow_color_val {};

    //! @brief Declaration tag associating to storage tags regulating the size of node shadows (defaults to none).
    template <typename T>
    struct shadow_size_tag {};

    //! @brief Declaration tag associating to the base size of node shadows (defaults to 0).
    template <intmax_t num, intmax_t den = 1>
    struct shadow_size_val {};

    //! @brief Declaration tag associating to storage tags regulating the time duration of past positions creating the node tail (defaults to none).
    template <typename T>
    struct tail_time_tag {};

    //! @brief Declaration tag associating to the time duration of past positions creating the node tail (defaults to 0).
    template <intmax_t num, intmax_t den = 1>
    struct tail_time_val {};

    //! @brief Declaration tag associating to the maximum granularity of snapshot points in tails in FPS (defaults to \ref FCPP_TAIL_GRANULARITY).
    template <intmax_t num, intmax_t den = 1>
    struct tail_granularity {};

    //! @brief Declaration tag associating to storage tags regulating the color of the node tail (defaults to none).
    template <typename T>
    struct tail_color_tag {};

    //! @brief Declaration tag associating to the base color of the node tail (defaults to black).
    template <intmax_t c>
    struct tail_color_val {};

    //! @brief Declaration tag associating to storage tags regulating the width of the node tail (defaults to none).
    template <typename T>
    struct tail_width_tag {};

    //! @brief Declaration tag associating to the width of the node tail as fraction of node size (defaults to 1).
    template <intmax_t num, intmax_t den = 1>
    struct tail_width_val {};

    //! @brief Declaration tag associating to the colors of the general theme (defaults to white/black/cyan).
    template <intmax_t background, intmax_t foreground, intmax_t selection>
    struct color_theme {};

    //! @brief Declaration tag associating to the bounding coordinates of the grid area (defaults to the minimal area covering initial nodes)
    template <intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax, intmax_t den = 1>
    struct area;

    //! @brief Declaration tag associating to the antialiasing factor (defaults to \ref FCPP_ANTIALIAS).
    template <intmax_t n>
    struct antialias {};

    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel;

    //! @brief Net initialisation tag associating to the minimum coordinates of the grid area (defaults to the value in \ref tags::area).
    struct area_min {};

    //! @brief Net initialisation tag associating to the maximum coordinates of the grid area (defaults to the value in \ref tags::area).
    struct area_max {};

    //! @brief Net initialisation tag associating to the main name of a component composition instance (defaults to the empty string).
    struct name;

    //! @brief Net initialisation tag associating to the refresh rate (0 for opportunistic frame refreshing, defaults to \ref FCPP_REFRESH_RATE).
    struct refresh_rate {};

    //! @brief Net initialisation tag associating to the texture to be used for the reference plane (defaults to none).
    struct texture {};

    //! @brief Net initialisation tag associating to the number of threads that can be created (defaults to \ref FCPP_THREADS).
    struct threads;
}


#ifdef FCPP_GUI
//! @brief Class displaying detailed node information in a separate window.
template <typename F>
class info_window {
    //! @brief The local part of the component.
    using node = typename F::node;
    //! @brief The global part of the component.
    using net = typename F::net;

  public:
    //! @brief Constructor.
    info_window(net& n, std::vector<device_t> uid) :
        m_net(n),
        m_uid(uid),
        m_renderer(0, get_title(), false, n.getRenderer().getWindow()),
        m_thread(&info_window::draw_cycle, this) {
        m_spacing = m_uid.size() == 1 ? 200 : std::max(70 - 10*(int)m_uid.size(), 15);
        using U = std::decay_t<decltype(m_net.node_at(0).storage_tuple())>;
        init_storage_values(m_keys, (typename U::tags){});
        init_storage_values(m_types, (typename U::types){});
        std::vector<std::string> vk{
            "",
            "previous_time",
            "current_time",
            "next_time",
            "frequency",
            "message_time",
            "",
            "position",
            "velocity",
            "propulsion",
            "acceleration",
            "friction",
            "nbr_dist",
            "nbr_vec",
            "connector_data"
        };
        m_keys.insert(m_keys.end(), vk.begin(), vk.end());
        std::sort(m_uid.begin(), m_uid.end());
        size_t ml = 0;
        for (auto const& s : m_keys) {
            ml = std::max(ml, s.size());
            m_values.emplace_back(m_uid.size());
        }

        for (int j=0; j<m_uid.size(); ++j) {
            while (m_nodes_list.size() < ml + m_spacing * j + 1) m_nodes_list.push_back(' ');
            m_nodes_list += std::to_string(m_uid[j]);
        }
        for (auto& s : m_keys) while (s.size() <= ml) s.push_back(' ');
        for (auto const& s : vk)
            m_types.emplace_back();

        for (int i : m_uid) if (m_net.node_count(i)) {
            typename net::lock_type l;
            node& n = m_net.node_at(i, l);
            update_values(n);
            n.highlight(2);
            n.set_window(this);
        }
    }

    //! @brief Copy constructor.
    info_window(info_window const&) = delete;

    //! @brief Move constructor.
    info_window(info_window&&) = delete;

    //! @brief Destructor.
    ~info_window() {
        m_running = false;
        m_thread.join();
        for (int i : m_uid) if (m_net.node_count(i)) {
            typename net::lock_type l;
            node& n = m_net.node_at(i, l);
            n.highlight(0);
            n.set_window(nullptr);
        }
    }

    //! @brief Whether the window should be closed.
    bool closing() {
        return glfwWindowShouldClose(m_renderer.getWindow());
    }

    //! @brief Sets the window as modified.
    void set_modified() {
        m_modified = true;
    }

    //! @brief Updates values.
    void update_values(node& n) {
        m_modified = true;
        int j = std::lower_bound(m_uid.begin(), m_uid.end(), n.uid) - m_uid.begin();
        update_values(
            j,
            fcpp::details::get_context(n).second().align(n.uid),
            n.storage_tuple(),
            "#@>---<@#",
            n.previous_time(),
            n.current_time(),
            n.next_time(),
            n.frequency(),
            n.message_time(),
            "#@>---<@#",
            n.position(),
            n.velocity(),
            n.propulsion(),
            n.acceleration(),
            n.friction(),
            n.nbr_dist(),
            n.nbr_vec(),
            n.connector_data()
        );
    }

 private:
    //! @brief Produces a title given the list of UIDs.
    std::string get_title() {
        if (m_uid.size() == 1) return "node " + std::to_string(m_uid[0]);
        std::string s = "nodes " + std::to_string(m_uid[0]);
        for (int i=1; i<m_uid.size(); ++i) s += ", " + std::to_string(m_uid[i]);
        return s;
    }

    //! @brief It sets the window resize callback.
    void setResizeCallback() {
        // Associates this (the info_window instance) to m_window
        glfwSetWindowUserPointer(m_renderer.getWindow(), this);

        // Viewport callback
        glfwSetFramebufferSizeCallback(m_renderer.getWindow(), [](GLFWwindow* window, int width, int height) {
            info_window& info = *((info_window*)glfwGetWindowUserPointer(window)); // get the info_window instance from window
            int winWidth, winHeight;
            glfwGetWindowSize(window, &winWidth, &winHeight);
            info.m_renderer.viewportResize(winWidth, winHeight, width, height);
            info.set_modified();
        });
    }

    //! @brief Main cycle.
    void draw_cycle() {
        m_renderer.initializeContext(false);
        setResizeCallback();
        while (m_running) {
            if (m_modified) {
                draw();
                m_modified = false;
                m_renderer.swapAndNext();
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            else std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        glfwMakeContextCurrent(NULL);
    }

    //! @brief Updates the node info and draws it into the window.
    void draw() {
        if (m_uid.size() == 1 and m_net.node_count(m_uid[0]) == 0)
            glfwSetWindowTitle(m_renderer.getWindow(), ("node " + std::to_string(m_uid[0]) + " (terminated)").c_str());
        int offs = 0;
        if (m_uid.size() > 1) {
            float y = (1 - (0.5f) / (m_keys.size()+2)) * (m_renderer.getWindowHeight());
            m_renderer.drawText(m_nodes_list, 0.16f, y, 0.25f);
            offs = 2;
        }
        for (size_t i=0; i<m_keys.size(); ++i) {
            std::string s = m_keys[i];
            for (size_t j=0; j<m_uid.size(); ++j) {
                while (s.size() < m_keys[i].size() + m_spacing * j) s.push_back(' ');
                if (m_values[i][j].size() >= m_spacing) {
                    m_values[i][j].resize(m_spacing - 4);
                    m_values[i][j] += "...";
                }
                s += m_values[i][j];
            }
            float y = (1 - (i+offs+0.5f) / (m_keys.size()+offs)) * (m_renderer.getWindowHeight());
            m_renderer.drawText(s, 0.16f, y, 0.25f);
        }
    }

    //! @brief Writes the tags from the tagged tuple.
    inline void init_storage_values(std::vector<std::string>&, common::type_sequence<>) {}
    template <typename S, typename... Ss>
    inline void init_storage_values(std::vector<std::string>& v, common::type_sequence<S, Ss...>) {
        v.push_back(common::strip_namespaces(common::type_name<S>()));
        init_storage_values(v, common::type_sequence<Ss...>{});
    }

    //! @brief Compile-time check whether type T is convertible to string (base case).
    template <typename T, typename = void>
    struct is_stringable : std::is_convertible<T,std::string> {};

    //! @brief Compile-time check whether type T is convertible to string.
    template <typename T>
    struct is_stringable<T,
        typename std::enable_if_t<
            std::is_same<std::decay_t<decltype(to_string(std::declval<T>()))>, std::string>::value
        >
    > : std::true_type {};

    //! @brief Compile-time check whether type T is printable (base case).
    template <typename T, typename = void>
    struct is_printable : is_stringable<T> {};

    //! @brief Compile-time check whether type T is printable.
    template <typename T>
    struct is_printable<T,
        typename std::enable_if_t<
            std::is_base_of<std::decay_t<decltype(std::declval<std::stringstream&>() << std::declval<T>())>, std::stringstream>::value
        >
    > : std::true_type {};

    //! @brief Converts value to string (string case).
    inline std::string stringify(std::string const& x, std::string const&) {
        return x == "#@>---<@#" ? "" : "\"" + x + "\"";
    }

    //! @brief Converts value to string (stringable case).
    template <typename T>
    inline std::enable_if_t<is_stringable<T>::value and not std::is_convertible<T,std::string>::value, std::string>
    stringify(T const& x, std::string const&) {
        return to_string(x);
    }

    //! @brief Converts value to string (printable case).
    template <typename T>
    inline std::enable_if_t<is_printable<T>::value and not is_stringable<T>::value, std::string>
    stringify(T const& x, std::string const&) {
        std::stringstream ss;
        ss << x;
        return ss.str();
    }

    //! @brief Converts value to string (non-printable case).
    template <typename T>
    inline std::enable_if_t<not is_printable<T>::value, std::string>
    stringify(T const& x, std::string s) {
        s.push_back('(');
        s.push_back('0');
        s.push_back('x');
        size_t y = (size_t)(&x);
        for (int i = sizeof(size_t)*CHAR_BIT-4; i >= 0; i-=4) {
            char z = (y>>i)&15;
            s.push_back(z < 10 ? z+'0' : z-10+'a');
        }
        s.push_back(')');
        return s;
    }

    //! @brief Updates values (internal).
    template <typename... Ss, typename T, typename... Ts>
    void update_values(size_t j, std::vector<device_t> n, common::tagged_tuple<common::type_sequence<Ss...>,T> const& t, Ts const&... xs) {
        update_values(j, n, 0, common::get<Ss>(t)..., xs...);
    }
    inline void update_values(size_t, std::vector<device_t> const&, size_t) {}
    template <typename T, typename... Ts>
    inline void update_values(size_t j, std::vector<device_t> const& n, size_t i, T const& x, Ts const&... xs) {
        m_values[i][j] = stringify(fcpp::details::align(x, n), m_types[i]);
        update_values(j, n, i+1, xs...);
    }

    //! @brief A reference to the corresponding net object.
    net& m_net;

    //! @brief Node represented.
    std::string m_nodes_list = "uid";

    //! @brief The length of each column.
    int m_spacing;

    //! @brief The unique identifier of the displayed device.
    std::vector<device_t> m_uid;

    //! @brief Window renderer object.
    internal::renderer m_renderer;

    //! @brief Whether the window is alive and running.
    bool m_running = true;

    //! @brief Whether the contents of the window have been modified.
    bool m_modified = false;

    //! @brief Rendering thread.
    std::thread m_thread;

    //! @brief Keys to be represented.
    std::vector<std::string> m_keys;

    //! @brief Values to be represented.
    std::vector<std::vector<std::string>> m_values;

    //! @brief Types of the values represented.
    std::vector<std::string> m_types;
};


//! @cond INTERNAL
namespace details {
    //! @brief Converts a number sequence to a vec (general form).
    template <typename T>
    struct numseq_to_vec;
    //! @brief Converts a number sequence to a vec (empty form).
    template <>
    struct numseq_to_vec<common::number_sequence<>> {
        constexpr static vec<0> min{};
        constexpr static vec<0> max{};
    };
    //! @brief Converts a number sequence to a vec (active form).
    template <intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax, intmax_t den>
    struct numseq_to_vec<common::number_sequence<xmin,ymin,xmax,ymax,den>> {
        constexpr static vec<2> min{xmin*1.0/den,ymin*1.0/den};
        constexpr static vec<2> max{xmax*1.0/den,ymax*1.0/den};
    };
    //! @brief Converts a vec to a glm 3D vector, filling missing coordinates with x.
    template <size_t n>
    inline glm::vec3 vec_to_glm(vec<n> v, float x) {
        glm::vec3 w(x, x, x);
        for (size_t i=0; i<n; ++i) w[i] = v[i];
        return w;
    }
}
//! @endcond


/**
 * @brief Component representing the simulation status graphically.
 *
 * Requires a \ref identifier , \ref simulated_positioner "positioner", \ref storage and \ref timer parent component.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::shape_tag defines a storage tag regulating the shape of nodes (defaults to none).
 * - \ref tags::shape_val defines the base shape of nodes (defaults to cube).
 * - \ref tags::size_tag defines a storage tag regulating the size of nodes (defaults to none).
 * - \ref tags::size_val defines the base size of nodes (defaults to 1).
 * - \ref tags::color_tag defines storage tags regulating the colors of nodes (defaults to none).
 * - \ref tags::color_val defines the base colors of nodes (defaults to white).
 * - \ref tags::label_text_tag defines a storage tag regulating the text of node labels (defaults to no text).
 * - \ref tags::label_size_tag defines a storage tag regulating the size of node labels (defaults to none).
 * - \ref tags::label_size_val defines the base size of node labels (defaults to 1).
 * - \ref tags::label_color_tag defines a storage tag regulating the color of node labels (defaults to none).
 * - \ref tags::label_color_val defines the base color of node labels (defaults to black).
 * - \ref tags::shadow_shape_tag defines a storage tag regulating the shape of node shadows (defaults to none).
 * - \ref tags::shadow_shape_val defines the base shape of node shadows (defaults to the same shape as the node).
 * - \ref tags::shadow_color_tag defines a storage tag regulating the color of node shadows (defaults to none).
 * - \ref tags::shadow_color_val defines the base color of node shadows (defaults to the same color as the node).
 * - \ref tags::shadow_size_tag defines a storage tag regulating the size of node shadows (defaults to none).
 * - \ref tags::shadow_size_val defines the base size of node shadows (defaults to 0).
 * - \ref tags::tail_time_tag defines a storage tag regulating the time duration of past positions creating the node tail (defaults to none).
 * - \ref tags::tail_time_val defines the base time duration of past positions creating the node tail (defaults to 0).
 * - \ref tags::tail_granularity defines the maximum granularity of snapshot points in tails in FPS (defaults to \ref FCPP_TAIL_GRANULARITY).
 * - \ref tags::tail_color_tag defines a storage tag regulating the color of the node tail (defaults to none).
 * - \ref tags::tail_color_val defines the base color of the node tail (defaults to black).
 * - \ref tags::tail_width_tag defines a storage tag regulating the width of the node tail (defaults to none).
 * - \ref tags::tail_width_val defines the base width of the node tail as fraction of node size (defaults to 1).
 * - \ref tags::color_theme defines the colors of the general theme (defaults to white/black/cyan).
 * - \ref tags::area defines the bounding coordinates of the grid area (defaults to the minimal area covering initial nodes).
 * - \ref tags::antialias defines the antialiasing factor (defaults to \ref FCPP_ANTIALIAS).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::area_min associates to the minimum coordinates of the grid area (defaults to the value in \ref tags::area).
 * - \ref tags::area_max associates to the maximum coordinates of the grid area (defaults to the value in \ref tags::area).
 * - \ref tags::name associates to the main name of a component composition instance (defaults to the empty string).
 * - \ref tags::refresh_rate associates to the refresh rate (0 for opportunistic frame refreshing, defaults to \ref FCPP_REFRESH_RATE).
 * - \ref tags::texture associates to the texture to be used for the reference plane (defaults to none).
 * - \ref tags::threads associates to the number of threads that can be created (defaults to \ref FCPP_THREADS).
 *
 * If no color tags or color values are specified, the color defaults to black.
 */
template <class... Ts>
struct displayer {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Colors of the general theme.
    using color_theme = common::option_nums<tags::color_theme, Ts...>;

    static_assert(color_theme::size == 3 or color_theme::size == 0, "the colors of a theme must be 3 integers");

    //! @brief Bounding coordinates of the grid area.
    using area = common::option_nums<tags::area, Ts...>;

    static_assert(area::size == 5 or area::size == 0, "the bounding coordinates must be 4 integers");

    //! @brief Antialiasing factor.
    constexpr static intmax_t antialias = common::option_num<tags::antialias, FCPP_ANTIALIAS, Ts...>;

    //! @brief Storage tag regulating the shape of nodes.
    using shape_tag = common::option_type<tags::shape_tag, void, Ts...>;

    //! @brief Base shape of nodes (defaults to cube).
    constexpr static shape shape_val = static_cast<shape>(common::option_num<tags::shape_val, static_cast<intmax_t>(shape::cube), Ts...>);

    //! @brief Storage tag regulating the size of nodes.
    using size_tag = common::option_type<tags::size_tag, void, Ts...>;

    //! @brief Base size of nodes (defaults to 1).
    constexpr static double size_val = common::option_float<tags::size_val, 1, 1, Ts...>;

    //! @brief Storage tags regulating the colors of nodes.
    using color_tag = common::option_types<tags::color_tag, Ts...>;

    //! @brief Base colors of nodes (defaults to white).
    using color_val = common::option_nums<tags::color_val, Ts...>;

    //! @brief Storage tag associated to the text of the node labels.
    using label_text_tag = common::option_type<tags::label_text_tag, void, Ts...>;

    //! @brief Storage tag regulating the size of node labels.
    using label_size_tag = common::option_type<tags::label_size_tag, void, Ts...>;

    //! @brief Base size of node labels (defaults to 1).
    constexpr static double label_size_val = common::option_float<tags::label_size_val, 1, 1, Ts...>;

    //! @brief Storage tag regulating the color of node labels.
    using label_color_tag = common::option_type<tags::label_color_tag, void, Ts...>;

    //! @brief Base color of node labels (defaults to black).
    constexpr static intmax_t label_color_val = common::option_num<tags::label_color_val, BLACK, Ts...>;

    //! @brief Storage tag regulating the shape of node labels.
    using shadow_shape_tag = common::option_type<tags::shadow_shape_tag, void, Ts...>;

    //! @brief Base shape of nodes (defaults to the same shape as the node).
    constexpr static intmax_t shadow_shape_val = common::option_num<tags::shadow_shape_val, -1, Ts...>;

    //! @brief Storage tags regulating the color of node shadows.
    using shadow_color_tag = common::option_type<tags::shadow_color_tag, void, Ts...>;

    //! @brief Base colors of node shadows (defaults to the same color as the node).
    constexpr static intmax_t shadow_color_val = common::option_num<tags::shadow_color_val, -1, Ts...>;

    //! @brief Storage tag regulating the size of node shadows.
    using shadow_size_tag = common::option_type<tags::shadow_size_tag, void, Ts...>;

    //! @brief Base size of node shadows (defaults to 0).
    constexpr static double shadow_size_val = common::option_float<tags::shadow_size_val, 0, 1, Ts...>;

    //! @brief Storage tag regulating the time duration of past positions creating the node tail.
    using tail_time_tag = common::option_type<tags::tail_time_tag, void, Ts...>;

    //! @brief Time duration of past positions creating the node tail (defaults to 0).
    constexpr static double tail_time_val = common::option_float<tags::tail_time_val, 0, 1, Ts...>;

    //! @brief Maximum granularity of snapshot points in tails in FPS (defaults to FCPP_TAIL_GRANULARITY).
    constexpr static double tail_granularity = 1.0 / common::option_float<tags::tail_granularity, FCPP_TAIL_GRANULARITY, 1, Ts...>;

    //! @brief Storage tag regulating the color of the node tail.
    using tail_color_tag = common::option_type<tags::tail_color_tag, void, Ts...>;

    //! @brief Base color of the node tail (defaults to \ref BLACK).
    constexpr static intmax_t tail_color_val = common::option_num<tags::tail_color_val, BLACK, Ts...>;

    //! @brief Storage tag regulating the width of the node tail.
    using tail_width_tag = common::option_type<tags::tail_width_tag, void, Ts...>;

    //! @brief Base width of the node tail as fraction of node size (defaults to 1).
    constexpr static double tail_width_val = common::option_float<tags::tail_width_val, 1, 1, Ts...>;

    //! @brief Whether there are labels to be drawn.
    constexpr static bool has_label = not std::is_same<label_text_tag, void>::value and (not std::is_same<label_size_tag, void>::value or label_size_val > 0);

    //! @brief Whether there are tails to be drawn.
    constexpr static bool has_tail = tail_width_val > 0 and (not std::is_same<tail_time_tag, void>::value or tail_time_val != 0);

    //! @brief Whether the drawAlpha phase is needed.
    constexpr static bool has_transparency = has_label or has_tail;

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
        //! @cond INTERNAL
        REQUIRE_COMPONENT(displayer,identifier);
        REQUIRE_COMPONENT(displayer,positioner);
        REQUIRE_COMPONENT(displayer,storage);
        REQUIRE_COMPONENT(displayer,timer);
        //! @endcond

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
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_highlight(0), m_window(nullptr), m_nbr_uids(), m_prev_nbr_uids(), m_tail_color(tail_color_val) {
                m_colors.resize(std::max(size_t(1), color_val::size + color_tag::size));
                color_val_put(m_colors, common::number_sequence<0>{}, color_val{});
            }

            //! @brief Caches the current position for later use.
            glm::vec3 const& cache_position(times_t t) {
                m_position = to_vec3(P::node::position(t));
                if (m_tail_times.size() > 1 and t - m_tail_times[m_tail_times.size()-2] < tail_granularity) {
                    m_tail_points.pop_back();
                    m_tail_normals.pop_back();
                    m_tail_times.pop_back();
                }
                m_tail_points.push_back(m_position);
                m_tail_times.push_back(t);
                size_t s = m_tail_times.size();
                if (s > 2) {
                    vec<2> n{m_tail_points[s-3].y - m_tail_points[s-1].y, m_tail_points[s-1].x - m_tail_points[s-3].x};
                    if (n < 0.1) m_tail_normals.back() = {0,0};
                    else m_tail_normals.back() = unit(std::move(n));
                }
                if (s == 1) m_tail_normals.push_back({0,0});
                else {
                    vec<2> n{m_tail_points[s-2].y - m_tail_points[s-1].y, m_tail_points[s-1].x - m_tail_points[s-2].x};
                    if (n < 0.1) m_tail_normals.push_back({0,0});
                    else m_tail_normals.push_back(unit(std::move(n)));
                }
                double dt = common::get_or<tail_time_tag>(P::node::storage_tuple(), tail_time_val);
                while (m_tail_times.front() < t - dt) {
                    m_tail_points.pop_front();
                    m_tail_normals.pop_front();
                    m_tail_times.pop_front();
                }
                return m_position;
            }

            //! @brief Accesses the cached position.
            glm::vec3 const& get_cached_position() const {
                return m_position;
            }

            //! @brief Draws the opaque representation of the node component.
            void draw(bool star) const {
                // gather shape and size
                shape s = common::get_or<shape_tag>(P::node::storage_tuple(), shape(shape_val));
                double d = common::get_or<size_tag>(P::node::storage_tuple(), size_val);
                if (m_highlight) d *= 1.5;
                // gather personal position
                glm::vec3 p = get_cached_position();
                // render the node
                P::node::net.getRenderer().drawShape(s, p, d, m_colors);
                // render the shadow
                double shadow_d = common::get_or<shadow_size_tag>(P::node::storage_tuple(), shadow_size_val);
                if (shadow_d > 0) {
                    shape shadow_s = common::get_or<shadow_shape_tag>(P::node::storage_tuple(), shadow_shape_val == -1 ? s : shape(shadow_shape_val));
                    color shadow_c = common::get_or<shadow_color_tag>(P::node::storage_tuple(), shadow_color_val == -1 ? m_colors[0] : color(shadow_color_val));
                    P::node::net.getRenderer().drawShadow(shadow_s, p, shadow_d, {shadow_c.rgba[0], shadow_c.rgba[1], shadow_c.rgba[2], shadow_c.rgba[3]});
                }
                if (star) {
                    // gather neighbours' positions
                    std::vector<glm::vec3> np;
                    for (device_t d : m_prev_nbr_uids)
                        np.push_back(P::node::net.node_at(d).get_cached_position());
                    P::node::net.getRenderer().drawStar(p, np);
                }
            }

            //! @brief Draws the transparent representation of the node component.
            void drawAlpha() const {
                // render the label
                std::string label_text = common::get_or<label_text_tag>(P::node::storage_tuple(), "");
                if (label_text.size()) {
                    double d = common::get_or<size_tag>(P::node::storage_tuple(), size_val) * 0.5;
                    glm::vec3 p = get_cached_position() + glm::vec3(d,d,d);
                    double label_size = common::get_or<label_size_tag>(P::node::storage_tuple(), label_size_val);
                    color label_color = common::get_or<label_color_tag>(P::node::storage_tuple(), color(label_color_val));
                    P::node::net.getRenderer().drawLabel(label_text, p, {label_color.rgba[0], label_color.rgba[1], label_color.rgba[2], label_color.rgba[3]}, label_size);
                }
                if (m_tail_points.size() > 1) {
                    double d = common::get_or<size_tag>(P::node::storage_tuple(), size_val);
                    if (m_highlight) d *= 1.5;
                    d *= common::get_or<tail_width_tag>(P::node::storage_tuple(), tail_width_val);
                    P::node::net.getRenderer().drawTail(m_tail_points, m_tail_normals, m_tail_color, d);
                }
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                // update info window
                if (m_window != nullptr) m_window->update_values(*this);
                P::node::round_end(t);
                PROFILE_COUNT("displayer");
                // update color list
                color_tag_put(m_colors, common::number_sequence<color_val::size>{}, color_tag{});
                if (m_highlight) for (size_t j=color_val::size; j<color_val::size+color_tag::size; ++j)
                    for (size_t i=0; i<3; ++i) m_colors[j].rgba[i] = (m_colors[j].rgba[i]+1)/2;
                // update neighbours list
                std::sort(m_nbr_uids.begin(), m_nbr_uids.end());
                m_nbr_uids.erase(std::unique(m_nbr_uids.begin(), m_nbr_uids.end()), m_nbr_uids.end());
                m_prev_nbr_uids = std::move(m_nbr_uids);
                m_nbr_uids.clear();
                maybe_set_tail_color(P::node::storage_tuple(), common::number_sequence<std::is_same<tail_color_tag, void>::value>{});
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, common::tagged_tuple<S,T> const& m) {
                P::node::receive(t, d, m);
                m_nbr_uids.push_back(d);
            }

            //! @brief Sets the highlighted state for the node.
            void highlight(int highVal) {
                // Clamp highVal
                if (highVal < 0) highVal = 0;
                if (highVal > 2) highVal = 2;

                // Assign highVal
                if (m_highlight == 0 and highVal)
                    for (color& c : m_colors) for (size_t i=0; i<3; ++i) c.rgba[i] = (c.rgba[i]+1)/2;
                if (highVal == 0 and m_highlight)
                    for (color& c : m_colors) for (size_t i=0; i<3; ++i) c.rgba[i] = c.rgba[i]*2-1;
                m_highlight = highVal;
            }

            //! @brief It returns the highlight state of the node.
            int get_highlight() const {
                return m_highlight;
            }

            //! @brief Sets the info window for the node.
            void set_window(info_window<F>* w = nullptr) {
                m_window = w;
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

            //! @brief Pushes colors in a number sequence into a vector (base case).
            template <intmax_t i>
            inline void color_val_put(std::vector<color>&, common::number_sequence<i>, common::number_sequence<>) const {}

            //! @brief Pushes colors in a number sequence into a vector (inductive case).
            template <intmax_t i, intmax_t x, intmax_t... xs>
            inline void color_val_put(std::vector<color>& c, common::number_sequence<i>, common::number_sequence<x, xs...>) const {
                c[i] = color(x);
                color_val_put(c, common::number_sequence<i+1>{}, common::number_sequence<xs...>{});
            }

            //! @brief Pushes colors from storage tags into a vector (base case).
            template <intmax_t i>
            inline void color_tag_put(std::vector<color>&, common::number_sequence<i>, common::type_sequence<>) const {}

            //! @brief Pushes colors from storage tags into a vector (inductive case).
            template <intmax_t i, typename S, typename... Ss>
            inline void color_tag_put(std::vector<color>& c, common::number_sequence<i>, common::type_sequence<S, Ss...>) const {
                c[i] = P::node::storage(S{});
                color_tag_put(c, common::number_sequence<i+1>{}, common::type_sequence<Ss...>{});
            }

            //! @brief Does not set the tail color, if there are no tails.
            template <typename T>
            inline void maybe_set_tail_color(T const&, common::number_sequence<true>) {}

            //! @brief Sets the tail color, if there are tails.
            template <typename T>
            inline void maybe_set_tail_color(T const& t, common::number_sequence<false>) {
                m_tail_color = common::get<tail_color_tag>(t);
            }

            //! @brief Whether the node is highlighted.
            //! 0 - not highlighted
            //! 1 - yes, with cursor hovering
            //! 2 - yes, with info window open
            int m_highlight;

            //! @brief Reference to the info window (if present).
            info_window<F>* m_window;

            //! @brief The current position of the device.
            glm::vec3 m_position;

            //! @brief The uids of incoming messages.
            std::vector<device_t> m_nbr_uids;

            //! @brief The uids of incoming messages during the previous round.
            std::vector<device_t> m_prev_nbr_uids;

            //! @brief The list of colors for the node.
            std::vector<color> m_colors;

            //! @brief The color of the tail.
            color m_tail_color;

            //! @brief The type name for node labels.
            std::string m_label_type;

            //! @brief The vector of points comprising the tail.
            std::deque<glm::vec3> m_tail_points;

            //! @brief The vector of vectors defining the tail width.
            std::deque<vec<2>> m_tail_normals;

            //! @brief The vector of times for points in the tail.
            std::deque<times_t> m_tail_times;
        };

        //! @brief The global part of the component.
        class net : public P::net {
        public: // visible by node objects and the main program
          //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) :
                P::net{ t },
                m_threads( common::get_or<tags::threads>(t, FCPP_THREADS) ),
                m_refresh{ 0 },
                m_step( common::get_or<tags::refresh_rate>(t, FCPP_REFRESH_RATE) ),
                m_rayCast{ 0.0, 0.0, 0.0 },
                m_renderer{antialias, common::get_or<tags::name>(t, "FCPP")},
                m_mouseLastX{ 0.0f },
                m_mouseLastY{ 0.0f },
                m_mouseRightX{ 0.0f },
                m_mouseRightY{ 0.0f },
                m_mouseFirst{ 1 },
                m_mouseRight{ 0 },
                m_links{ false },
                m_pointer{ true },
                m_legenda{ false },
                m_texture( common::get_or<tags::texture>(t, "") ),
                m_deltaTime{ 0.0f },
                m_lastFrame{ 0.0f },
                m_lastFraction{ 0.0f },
                m_FPS{ 0 } {
                    m_frameCounts.push_back(0);
                    constexpr auto max = details::numseq_to_vec<area>::max;
                    constexpr auto min = details::numseq_to_vec<area>::min;
                    m_viewport_max = details::vec_to_glm(common::get_or<tags::area_max>(t, max), -INF);
                    m_viewport_min = details::vec_to_glm(common::get_or<tags::area_min>(t, min), +INF);
                    maybe_set_color_theme(color_theme{});
                }

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                times_t nxt = P::net::next();
                if (nxt == TIME_MAX and P::net::frequency() > 0) return TIME_MAX;
                return std::min(std::min(m_refresh, P::net::real_time()), nxt);
            }

            //! @brief Updates the internal status of net component.
            void update() {
                times_t rt = std::min(m_refresh, P::net::real_time());
                if (rt < P::net::next()) {
                    times_t t = P::net::internal_time();
                    auto n_beg = P::net::node_begin();
                    auto n_end = P::net::node_end();
                    PROFILE_COUNT("displayer");
                    if (not m_legenda) {
                        PROFILE_COUNT("displayer/nodes");
                        if (rt == 0) {
                            if (m_viewport_min.x > m_viewport_max.x) {
                                common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&] (size_t i, size_t) {
                                    viewport_update(n_beg[i].second.cache_position(t));
                                });
                                double approx = 1;
                                while ((m_viewport_max.x - m_viewport_min.x) * (m_viewport_max.y - m_viewport_min.y) > 2000 * approx * approx)
                                    approx *= 10;
                                while ((m_viewport_max.x - m_viewport_min.x) * (m_viewport_max.y - m_viewport_min.y) <= 20 * approx * approx)
                                    approx /= 10;
                                m_viewport_min.x = std::floor(m_viewport_min.x / approx) * approx;
                                m_viewport_max.x = std::ceil(m_viewport_max.x / approx) * approx;
                                m_viewport_min.y = std::floor(m_viewport_min.y / approx) * approx;
                                m_viewport_max.y = std::ceil(m_viewport_max.y / approx) * approx;
                            } else m_viewport_min[2] = m_viewport_max[2] = 0;
                        } else {
                            if (m_pointer && m_mouseStartX == std::numeric_limits<float>::infinity()) highlightHoveredNode();
                            common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&] (size_t i, size_t) {
                                n_beg[i].second.cache_position(t);
                            });
                        }
                        for (size_t i = 0; i < n_end-n_beg; ++i) n_beg[i].second.draw(m_links);
                    }
                    if (rt == 0) {
                        // stop simulated time
                        P::net::frequency(0);
                        // first frame only: set camera position, rotation, sensitivity
                        glm::vec3 viewport_size = m_viewport_max - m_viewport_min;
                        glm::vec3 camera_pos = (m_viewport_min + m_viewport_max) / 2.0f;
                        double dz = std::max(viewport_size.x/m_renderer.getAspectRatio(), viewport_size.y);
                        dz /= tan(45.0f / 2) * 1.4;
                        camera_pos.z = dz;
                        m_renderer.setLightPosition(camera_pos);
                        m_renderer.setStandardCursor(m_pointer);
                        m_renderer.getCamera().setViewDefault(camera_pos, dz, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
                        double diagonal = glm::length(viewport_size);
                        double grid_scale = 1;
                        while (grid_scale * 200 < diagonal) grid_scale *= 10;
                        while (grid_scale * 20 > diagonal) grid_scale /= 10;
                        m_renderer.makeGrid(m_viewport_min, m_viewport_max, grid_scale);
                        m_renderer.setGridTexture(m_texture);
                        setInternalCallbacks(); // call this after m_renderer is initialized
                    }
                    if (not m_legenda) {
                        PROFILE_COUNT("displayer/grid");
                        // Draw grid
                        m_renderer.drawGrid(m_texture == "" ? 0.3f : 1.0f);
                        // Draw labels and tails
                        if (has_transparency) for (size_t i = 0; i < n_end-n_beg; ++i) n_beg[i].second.drawAlpha();
                    }
                    if (m_mouseStartX != std::numeric_limits<float>::infinity()) {
                        float sx{ (2.0f * (float)m_mouseStartX) / m_renderer.getFramebufferWidth() - 1.0f };
                        float sy{ 1.0f - (2.0f * (float)m_mouseStartY) / m_renderer.getFramebufferHeight() };
                        float lx{ (2.0f * (float)m_mouseLastX) / m_renderer.getFramebufferWidth() - 1.0f };
                        float ly{ 1.0f - (2.0f * (float)m_mouseLastY) / m_renderer.getFramebufferHeight() };
                        dragSelect();
                        m_renderer.drawRectangle(sx, sy, lx, ly);
                    }
                    {
                        PROFILE_COUNT("displayer/text");
                        if (m_legenda) {
                            // todo
                            m_renderer.drawText("LEGENDA", m_renderer.getWindowWidth() / 2, m_renderer.getWindowHeight() - 32.0f, 0.5f);
                            std::vector<std::pair<std::string, std::string>> text = {
                                {"ESC",         "stop and exit the simulation"},
                                {"P",           "play/pause the simulation"},
                                {"I/O",         "slow-down/speed-up simulated time"},
                                {"L",           "show/hide connection links between nodes"},
                                {"G",           "show/hide the reference grid and node pins"},
                                {"M",           "enable/disable the marker for selecting nodes"},
                                {"left-click",  "open a window with selected node details"},
                                {"C",           "resets the camera to the starting position"},
                                {"A/D",         "move the camera left/right"},
                                {"W/S",         "move the camera up/down"},
                                {"Q/E",         "move the camera forward/backward"},
                                {"right-click", "drag to rotate the simulation plane"},
                                {"scroll",      "zoom in and out of the simulation plane"},
                                {"left-shift",  "added to camera commands for precision control"},
                                {"other keys",  "show/hide the command legenda"}
                            };
                            size_t ml = 0;
                            for (auto const& line : text) ml = std::max(ml, line.first.size());
                            ml += 2;
                            for (auto& line : text) while (line.first.size() < ml) line.first.push_back(' ');
                            for (size_t i = 0; i < text.size(); ++i) {
                                float y = (1 - (i+0.5f) / text.size()) * (m_renderer.getWindowHeight() - 64);
                                m_renderer.drawText(text[i].first + text[i].second, 16.0f, y, 0.25f);
                            }
                        } else {
                            // Draw hovered node, simulation time (t) and FPS
                            if (m_hoveredNode != device_t(-1)) m_renderer.drawText("Node " + std::to_string(m_hoveredNode), 16.0f, m_renderer.getWindowHeight() - 16.0f, 0.25f);
                            std::string tt = "Simulation time: " + std::to_string(t);
                            if (P::net::frequency() != 1 and P::net::frequency() != 0) {
                                tt += " (";
                                int f = P::net::frequency();
                                tt += std::to_string(f);
                                if (P::net::frequency() < 1) tt += "." + std::to_string(int((P::net::frequency() - f)*100));
                                else if (P::net::frequency() < 10) tt += "." + std::to_string(int((P::net::frequency() - f)*10));
                                tt += "x)";
                            }
                            m_renderer.drawText(tt, 16.0f, 16.0f, 0.25f);
                            m_renderer.drawText(std::to_string(m_FPS) + " FPS", m_renderer.getWindowWidth() - 60.0f, 16.0f, 0.25f);
                        }
                    }
                    {
                        PROFILE_COUNT("displayer/input");
                        // Update deltaTime
                        updateDeltaTime();
                        // Handle pressed keys
                        processStroked();
                    }
                    {
                        PROFILE_COUNT("displayer/step");
                        // Destroy closed info windows
                        for (size_t i=0; i<m_info.size(); ) {
                            if (m_info[i]->closing()) {
                                using std::swap;
                                swap(m_info[i], m_info.back());
                                m_info.pop_back();
                            } else ++i;
                        }
                        // Swap buffers and prepare for next frame to draw
                        m_renderer.swapAndNext();
                        // Update m_refresh
                        m_refresh = rt + m_step;
                    }
                } else P::net::update();
            }

            //! @brief Returns net's renderer object.
            internal::renderer const& getRenderer() {
                return m_renderer;
            }

            //! @brief Returns net's delta time.
            float const& getDeltaTime() {
                return m_deltaTime;
            }

            //! @brief Sets the color theme.
            inline void set_color_theme(color background, color foreground, color selection) {
                m_renderer.setColorTheme(background, foreground, selection);
            }

        private: // implementation details
            //! @brief Sets the color theme based on options (empty overload).
            inline void maybe_set_color_theme(common::number_sequence<>) {}

            //! @brief Sets the color theme based on options (active overload).
            template <intmax_t background, intmax_t foreground, intmax_t selection>
            inline void maybe_set_color_theme(common::number_sequence<background,foreground,selection>) {
                set_color_theme(color(background), color(foreground), color(selection));
            }

            //! @brief Updates the viewport adding a position to it.
            void viewport_update(glm::vec3 pos) {
                for (int i=0; i<3; ++i) {
                    if (pos[i] < m_viewport_min[i]) {
                        common::lock_guard<parallel> l(m_viewport_mutex);
                        m_viewport_min[i] = pos[i];
                    }
                    if (pos[i] > m_viewport_max[i]) {
                        common::lock_guard<parallel> l(m_viewport_mutex);
                        m_viewport_max[i] = pos[i];
                    }
                }
            }

            //! @brief It updates m_deltaTime and m_lastFrame
            void updateDeltaTime() {
                float currentFrame{ (float)P::net::real_time() };
                m_deltaTime = currentFrame - m_lastFrame;
                m_lastFrame = currentFrame;
                constexpr int refresh = 4; // how many FPS refreshes per second
                while (currentFrame > m_lastFraction + 1.0f/refresh) {
                    m_FPS += m_frameCounts.back();
                    m_frameCounts.push_back(0);
                    m_lastFraction += 1.0f/refresh;
                }
                ++m_frameCounts.back();
                while (m_frameCounts.size() > refresh+1) {
                    m_FPS -= m_frameCounts.front();
                    m_frameCounts.pop_front();
                }
            }

            //! @brief It binds internally-defined callback functions to OpenGL events.
            void setInternalCallbacks() {
                // Associates this (the net instance) to m_window
                glfwSetWindowUserPointer(m_renderer.getWindow(), this);

                // Viewport callback
                glfwSetFramebufferSizeCallback(m_renderer.getWindow(), [](GLFWwindow* window, int width, int height) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    int winWidth, winHeight;
                    glfwGetWindowSize(window, &winWidth, &winHeight);
                    dspl.m_renderer.viewportResize(winWidth, winHeight, width, height);
                });

                // Keyboard callback
                glfwSetKeyCallback(m_renderer.getWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window

                    if (action == GLFW_PRESS) {
                        dspl.m_key_stroked.insert(key);
                        dspl.keyboardInput(key, true, 0, mods); // set deltaTime to 0?
                    }
                    else if (action == GLFW_RELEASE) {
                        dspl.m_key_stroked.erase(key);
                    }
                });

                // Cursor position callback
                glfwSetCursorPosCallback(m_renderer.getWindow(), [](GLFWwindow* window, double xpos, double ypos) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    xpos *= dspl.m_renderer.getRenderScale();
                    ypos *= dspl.m_renderer.getRenderScale();

                    if (dspl.m_mouseFirst) {
                        dspl.m_mouseLastX = (float)xpos;
                        dspl.m_mouseLastY = (float)ypos;
                        dspl.m_mouseFirst = false;
                    }

                    float xoffset{ (float)(xpos - dspl.m_mouseLastX) };
                    float yoffset{ (float)(dspl.m_mouseLastY - ypos) };
                    dspl.m_mouseLastX = (float)xpos;
                    dspl.m_mouseLastY = (float)ypos;

                    dspl.mouseInput(dspl.m_mouseLastX, dspl.m_mouseLastY, 0.0, 0.0, mouse_type::hover, 0);
                    dspl.mouseInput(xoffset, yoffset, 0.0, 0.0, mouse_type::drag, 0);
                });

                // Cursor click callback
                glfwSetMouseButtonCallback(m_renderer.getWindow(), [](GLFWwindow* window, int button, int action, int mods) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    dspl.mouseInput(dspl.m_mouseLastX, dspl.m_mouseLastY, 0.0, 0.0, mouse_type::click, mods);
                });

                // Cursor scroll callback
                glfwSetScrollCallback(m_renderer.getWindow(), [](GLFWwindow* window, double xoffset, double yoffset) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    dspl.mouseInput(xoffset, yoffset, 0.0, 0.0, mouse_type::scroll, 0);
                });

                // Window close callback
                glfwSetWindowCloseCallback(m_renderer.getWindow(), [](GLFWwindow* window) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    if (dspl.frequency() == 0) dspl.frequency(1);
                    dspl.terminate();
                });
            }

            //! @brief It returns the projection of a vector v onto a ray along its direction d (unit vector).
            inline glm::vec3 rayProjection(glm::vec3 const& v, glm::vec3 const& d) {
                return glm::dot<3, float, glm::qualifier::highp>(v, d) * d;
            }

            //! @brief It checks if the ray of direction d (unit vector) and position p intersects with a sphere of radius r at position c; length of projection of c onto ray is stored into prj.
            bool intersectSphere(glm::vec3 const& p, glm::vec3 const& d, float r, glm::vec3 const& c, float& prj) {
                bool intersection{ false };

                // Define vector from p to c
                glm::vec3 vpc{ c - p };

                // If the distance to the ray can be computed (the sphere isn't behind the ray)...
                if (glm::dot<3, float, glm::qualifier::highp>(vpc, d) > 0.0f) {
                    // Define projection of c on ray
                    glm::vec3 pc{ rayProjection(vpc, d) }; // there's no need to translate such projection at position p
                    if (glm::length(vpc - pc) <= r) {
                        intersection = true;
                        prj = glm::length(pc);
                    } else prj = 0.0f;
                }

                return intersection;
            }

            //! @brief It highlights the node hovered by the cursor.
            void highlightHoveredNode() {
                // Highlighting the right node
                auto beg{ P::net::node_begin() };
                auto end{ P::net::node_end() };
                float minDist{ (float)INF };
                if (P::net::node_count(m_hoveredNode) and P::net::node_at(m_hoveredNode).get_highlight() == 1) {
                    typename P::net::lock_type l;
                    P::net::node_at(m_hoveredNode, l).highlight(0);
                }
                m_hoveredNode = -1;
                for (size_t i = 0; i < end - beg; ++i) {
                    float prj;
                    float r{ (float)(common::get_or<size_tag>(beg[i].second.storage_tuple(), float(size_val))) * 1.5f };
                    if (intersectSphere(m_renderer.getCamera().getPosition(), m_rayCast, r, beg[i].second.get_cached_position(), prj)) {
                        if (prj < minDist) {
                            minDist = prj;
                            m_hoveredNode = beg[i].second.uid;
                        }
                    }
                }
                if (P::net::node_count(m_hoveredNode) and P::net::node_at(m_hoveredNode).get_highlight() == 0) {
                    typename P::net::lock_type l;
                    P::net::node_at(m_hoveredNode, l).highlight(1);
                }
            }

            inline bool pointInsideRectangle(float x, float y) {
                return x >= std::min(m_mouseStartX,m_mouseLastX) and x <= std::max(m_mouseStartX,m_mouseLastX) and y >= std::min(m_mouseStartY,m_mouseLastY) and y <= std::max(m_mouseStartY,m_mouseLastY);
            }

            void dragSelect() {
                auto beg{ P::net::node_begin() };
                auto end{ P::net::node_end() };
                for (size_t i = 0; i < end - beg; ++i) {
                    glm::vec4 clipSpacePos = m_renderer.getCamera().getPerspective() * (m_renderer.getCamera().getView() * glm::vec4(beg[i].second.get_cached_position(), 1.0));
                    glm::vec3 ndcSpacePos{ clipSpacePos.x / clipSpacePos.w,clipSpacePos.y / clipSpacePos.w,clipSpacePos.z/clipSpacePos.w };
                    float x = (ndcSpacePos.x + 1.0 ) * (m_renderer.getFramebufferWidth() / 2.0);
                    float y = (-ndcSpacePos.y + 1.0) * (m_renderer.getFramebufferHeight() / 2.0);
                    int h = beg[i].second.get_highlight();
                    bool inRect = pointInsideRectangle(x,y);
                    if (not inRect == h) beg[i].second.highlight(inRect);
                }
            }

            //! @brief It manages mouse input of the given type.
            void mouseInput(double x, double y, double xFirst, double yFirst, mouse_type type, int mods) {
                switch (type) {
                    case mouse_type::hover: {
                        // Raycast caluclation
                        // (x, y) from screen space coordinates to NDC
                        float rayX{ (2.0f * (float)x) / m_renderer.getFramebufferWidth() - 1.0f };
                        float rayY{ 1.0f - (2.0f * (float)y) / m_renderer.getFramebufferHeight() };

                        // Ray vector goes from screen space to world space (backwards)
                        // Ray in clip space; camera position at (0,0,0)
                        glm::vec4 clipRay{ rayX, rayY, -1.0f, 1.0f };

                        // Applying inverse of projection matrix in order to go into view space
                        glm::vec4 viewRay{ (glm::vec4)(glm::affineInverse(m_renderer.getCamera().getPerspective()) * clipRay) };
                        viewRay.z = -1.0f;
                        viewRay.w = 0.0f;

                        // Applying inverse of view matrix in order to go into world space
                        glm::vec4 worldRay4{ glm::affineInverse(m_renderer.getCamera().getView()) * viewRay };
                        m_rayCast.x = worldRay4.x; m_rayCast.y = worldRay4.y; m_rayCast.z = worldRay4.z;
                        m_rayCast = glm::normalize(m_rayCast);

                        break;
                    }

                    case mouse_type::click: {
                        GLFWwindow* window{ m_renderer.getWindow() };
                        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                            if (P::net::node_count(m_hoveredNode) and P::net::node_at(m_hoveredNode).get_highlight() == 1) {
                                glfwMakeContextCurrent(NULL);
                                m_info.emplace_back(new info_window<F>(*this, {m_hoveredNode}));
                                glfwMakeContextCurrent(m_renderer.getWindow());
                            }
                            else if (m_mouseStartX == std::numeric_limits<float>::infinity()) {
                                m_mouseStartX = x;
                                m_mouseStartY = y;
                            }
                        }
                        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE and m_mouseStartX != std::numeric_limits<float>::infinity()) {
                            std::vector<device_t> nodes;
                            auto beg{ P::net::node_begin() };
                            auto end{ P::net::node_end() };
                            for (size_t i = 0; i < end - beg; ++i) {
                                device_t node = beg[i].second.uid;
                                int h = beg[i].second.get_highlight();
                                if (h == 1) {
                                    nodes.emplace_back(node);
                                }
                            }
                            if (nodes.size() > 0) {
                                glfwMakeContextCurrent(NULL);
                                m_info.emplace_back(new info_window<F>(*this, nodes));
                                glfwMakeContextCurrent(m_renderer.getWindow());
                            }
                            m_mouseStartX = std::numeric_limits<float>::infinity();
                            m_mouseStartY = std::numeric_limits<float>::infinity();
                        }
                        m_renderer.mouseInput(x, y, 0.0f, 0.0f, mouse_type::click, mods);
                        break;
                    }


                    case mouse_type::drag: {
                        GLFWwindow* window{ m_renderer.getWindow() };
                        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                            if (!m_mouseRight) {
                                m_mouseRight = 1;
                                m_mouseRightX = m_mouseLastX - (float)(m_renderer.getFramebufferWidth() / 2);
                                m_mouseRightY = (float)(m_renderer.getFramebufferHeight() / 2) - m_mouseLastY;
                            }
                            int mods = 0;
                            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                                mods |= GLFW_MOD_SHIFT;
                            m_renderer.mouseInput(x, y, m_mouseRightX, m_mouseRightY, mouse_type::drag, mods);
                        }
                        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
                            m_mouseRight = 0;
                            m_mouseRightX = 0.0f;
                            m_mouseRightY = 0.0f;
                        }
                        break;
                    }

                    case mouse_type::scroll: {
                        int mods = 0;
                        if (glfwGetKey(m_renderer.getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                            mods |= GLFW_MOD_SHIFT;
                        m_renderer.mouseInput(x, y, 0.0, 0.0, mouse_type::scroll, mods);
                        break;
                    }
                }
            }

            //! @brief Given the key stroke, the press status and a deltaTime, it manages keyboard input for the displayer and other classes.
            void keyboardInput(int key, bool first, float deltaTime, int mods) {
                // exit legenda with any key
                bool justoutoflegenda = false;
                if (m_legenda) {
                    if (first) {
                        m_legenda = false;
                        justoutoflegenda = true;
                        m_renderer.setStandardCursor(m_pointer);
                    } else return;
                }
                switch (key) {
                    // terminate program
                    case GLFW_KEY_ESCAPE:
                        if (P::net::frequency() == 0) P::net::frequency(1);
                        P::net::terminate();
                        break;
                    // show/hide links
                    case GLFW_KEY_L:
                        if (first) m_links = not m_links;
                        break;
                    // enable/disable marker pointer
                    case GLFW_KEY_M:
                        if (first) {
                            if (m_pointer) {
                                if (P::net::node_count(m_hoveredNode) and P::net::node_at(m_hoveredNode).get_highlight() == 1) {
                                    typename P::net::lock_type l;
                                    P::net::node_at(m_hoveredNode, l).highlight(0);
                                }
                                m_hoveredNode = -1;
                            }
                            m_pointer = not m_pointer;
                            m_renderer.setStandardCursor(m_pointer, !m_mouseRight);
                        }
                        break;
                    // play/pause simulation
                    case GLFW_KEY_P:
                        if (first) {
                            // play/pause simulation
                            real_t f = P::net::frequency();
                            P::net::frequency(f == 0 ? 1 : 0);
                        }
                        break;
                    // accelerate simulation
                    case GLFW_KEY_O:
                        P::net::frequency(pow(4.0, (mods & GLFW_MOD_SHIFT) > 0 ? deltaTime/5 : deltaTime)*P::net::frequency());
                        break;
                    // decelerate simulation
                    case GLFW_KEY_I:
                        P::net::frequency(pow(0.25, (mods & GLFW_MOD_SHIFT) > 0 ? deltaTime/5 : deltaTime)*P::net::frequency());
                        break;
                    default:
                        // pass key to renderer
                        if (not m_renderer.keyboardInput(key, first, deltaTime, mods) and first and not justoutoflegenda and key != GLFW_KEY_LEFT_SHIFT and key != GLFW_KEY_RIGHT_SHIFT) {
                            // unrecognised key: stop simulation for legenda
                            P::net::frequency(0);
                            m_hoveredNode = -1;
                            m_legenda = true;
                            m_renderer.setStandardCursor(false);
                        }
                }
            }

            //! @brief It calls keyboardInput for every key in m_key_stroked.
            void processStroked() {
                // Handle pressed keys
                int mods = 0;
                if (glfwGetKey(m_renderer.getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                    mods |= GLFW_MOD_SHIFT;
                for (int key : m_key_stroked)
                    keyboardInput(key, false, m_deltaTime, mods);
            }

            //! @brief The number of threads to be used.
            size_t const m_threads;

            //! @brief The next refresh time.
            times_t m_refresh;

            //! @brief The step between refresh times.
            times_t m_step;

            //! @brief Net's renderer object; it has the responsability of calling OpenGL functions.
            internal::renderer m_renderer;

            //! @brief The running node-info windows.
            std::vector<std::unique_ptr<info_window<F>>> m_info;

            //! @brief Start mouse X position when the right click is pressed.
            float m_mouseStartX = std::numeric_limits<double>::infinity();

            //! @brief Start mouse Y position when the right click is pressed.
            float m_mouseStartY = std::numeric_limits<double>::infinity();

            //! @brief Last mouse X position.
            float m_mouseLastX;

            //! @brief Last mouse Y position.
            float m_mouseLastY;

            //! @brief First mouse X position when the right click is pressed.
            float m_mouseRightX;

            //! @brief First mouse Y position when the right click is pressed.
            float m_mouseRightY;

            //! @brief It checks if it's the first mouse's input capture.
            bool m_mouseFirst;

            //! @brief It checks if the right click is pressed.
            bool m_mouseRight;

            //! @brief Whether links between nodes should be displayed.
            bool m_links;

            //! @brief Whether nodes can be selected with mouse click.
            bool m_pointer;

            //! @brief Whether to show the legenda.
            bool m_legenda;

            //! @brief The texture to be used for the reference plane.
            std::string m_texture;

            //! @brief Time between current frame and last frame.
            float m_deltaTime;

            //! @brief Time of last frame.
            float m_lastFrame;

            //! @brief Frame counters in the last fractions of second.
            std::deque<int> m_frameCounts;

            //! @brief Time of the last fraction of second.
            float m_lastFraction;

            //! @brief The currently estimated FPS.
            int m_FPS;

            //! @brief The node currently hovered by the cursor.
            device_t m_hoveredNode;

            //! @brief List of currently stroked keys.
            std::unordered_set<int> m_key_stroked;

            //! @brief Boundaries of the viewport.
            glm::vec3 m_viewport_min, m_viewport_max;

            //! @brief Vector representing the raycast direction (in world space) generated while moving the cursor.
            glm::vec3 m_rayCast;

            //! @brief A mutex for regulating access to the viewport boundaries.
            common::mutex<parallel> m_viewport_mutex;
        };
    };
};
#else
//! @brief Inert version of the displayer component.
template <class... Ts>
struct displayer {
    //! @brief The actual component.
    template <typename F, typename P>
    struct component : public P {
        //! @brief The local part of the component.
        using node = typename P::node;
        //! @brief The global part of the component.
        using net = typename P::net;
    };
};
#endif


}


}

#endif // FCPP_SIMULATION_DISPLAYER_H_
