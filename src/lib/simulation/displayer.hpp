// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file displayer.hpp
 * @brief Implementation of the `displayer` component representing the simulation status graphically.
 */

#ifndef FCPP_SIMULATION_DISPLAYER_H_
#define FCPP_SIMULATION_DISPLAYER_H_

#include <cassert>
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

#include "lib/component/base.hpp"
#include "lib/data/vec.hpp"
#include "lib/graphics/renderer.h"
#include "lib/graphics/shapes.h"



/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

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
                    if (c.empty()) c.push_back(0); // black if nothing else
                    {
                        common::unlock_guard<parallel> ul(P::node::mutex);
                        for (device_t d : m_prev_nbr_uids) {
                            common::unique_lock<parallel> l;
                            np.push_back(P::node::net.node_at(d, l).get_cached_position(m_refresh));
                        }
                        if (m_refresh == 0) P::node::net.viewport_update(p);
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

                        // Draw cube through net's Renderer
                        P::node::net.getRenderer().drawCube(p, c);
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
                m_viewport_max{ -1.0 / 0.0, -1.0 / 0.0, -1.0 / 0.0 },
                m_viewport_min{ 1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0 },
                m_renderer{} {
                // Set simulation refresh rate
                m_step = common::get_or<tags::refresh_rate>(t, FCPP_REFRESH_RATE) * common::get_or<tags::realtime_factor>(t, FCPP_REALTIME);
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
                    if (m_refresh == 0) {
                        // first frame only: set camera position, rotation, sensitivity
                        std::cout << "m_viewport_min = (" << m_viewport_min.x << ", " << m_viewport_min.y << ", " << m_viewport_min.z << ")\n";
                        std::cout << "m_viewport_max = (" << m_viewport_max.x << ", " << m_viewport_max.y << ", " << m_viewport_max.z << ")\n";
                        glm::vec3 viewport_size = m_viewport_max - m_viewport_min;
                        glm::vec3 camera_pos = (m_viewport_min + m_viewport_max) / 2.0f;
                        double dz = std::max(viewport_size.x/m_renderer.aspectRatio(), viewport_size.y);
                        dz /= tan(m_renderer.viewAngle() / 2) * 2;
                        camera_pos.z = m_viewport_max.z + dz;
                        // roll/pitch angles should be zero (why is there no "roll" angle in your code so far?)
                        // yaw should be so that the front of the camera is towards negative values of z (not clear to me where the zero angle starts)
                        double zFar = sqrt(dz * (dz + viewport_size.z)) * 32;
                        double zNear = zFar / 1024;
                        // zFar/zNear also regulates the cameraSensitivity to input (speed of changes)
                        // mousewheel changes zFar & zNear & cameraSensitivity (all proportionally)
                        m_renderer.setPosition(camera_pos);
                        m_renderer.setYaw(270.0f);
                        m_renderer.setFarPlane((float)zFar);
                        m_renderer.setNearPlane((float)zNear);
                    }
                    times_t t = get_warped(has_timer<P>{}, *this, m_refresh);
                    /**
                     * Do whatever global management you need to do here
                     * (drawing the scene, interpreting the user input).
                     * You may want to display the current simulation time t.
                     */

                    // Draw orthogonal axis
                    m_renderer.drawOrtho();

                    // Swap buffers and prepare for next frame to draw
                    m_renderer.swapAndNext();
                    
                    // Update m_refresh
                    m_refresh += m_step;
                } else P::net::update();
            }

            //! @brief Returns the refresh step in subjective node time.
            times_t refresh_step() const {
                return get_warped(has_timer<P>{}, *this, m_step);
            }

            //! @brief Returns net's Renderer object.
            fcpp::internal::Renderer getRenderer() {
                return m_renderer;
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

            //! @brief The next refresh time.
            times_t m_refresh;

            //! @brief The step between refresh times.
            times_t m_step;

            //! @brief Net's Renderer object; it has the responsability of calling OpenGL functions.
            fcpp::internal::Renderer m_renderer;

            //! @brief Boundaries of the viewport.
            glm::vec3 m_viewport_min, m_viewport_max;

            //! @brief A mutex for regulating access to the viewport boundaries.
            common::mutex<parallel> m_viewport_mutex;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_DISPLAYER_H_
