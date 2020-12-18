// Copyright © 2020 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.

/**
 * @file displayer.hpp
 * @brief Implementation of the `displayer` component representing the simulation status graphically.
 */

#ifndef FCPP_SIMULATION_DISPLAYER_H_
#define FCPP_SIMULATION_DISPLAYER_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <string.h>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>

#include "lib/common/algorithm.hpp"
#include "lib/component/base.hpp"
#include "lib/data/vec.hpp"
#include "lib/graphics/renderer.hpp"
#include "lib/graphics/shapes.hpp"
#include "lib/graphics/input_types.hpp"


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

    //! @brief Declaration tag associating to the antialiasing factor.
    template <size_t n>
    struct antialias {};

    //! @brief Declaration flag associating to whether parallelism is enabled.
    template <bool b>
    struct parallel;

    //! @brief Net initialisation tag associating to the refresh rate (0 for opportunistic frame refreshing).
    struct refresh_rate {};

    //! @brief Net initialisation tag associating to the number of threads that can be created.
    struct threads;
}


/**
 * @brief Component representing the simulation status graphically.
 *
 * Requires a \ref identifier , \ref positioner, \ref storage and \ref timer parent component.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::shape_tag defines a storage tag regulating the shape of nodes (defaults to none).
 * - \ref tags::shape_val defines the base shape of nodes (defaults to cube).
 * - \ref tags::size_tag defines a storage tag regulating the size of nodes (defaults to none).
 * - \ref tags::size_val defines the base size of nodes (defaults to 1).
 * - \ref tags::color_tag defines storage tags regulating the colors of nodes (defaults to none).
 * - \ref tags::color_val defines the base colors of nodes (defaults to none).
 * - \ref tags::antialias defines the the antialiasing factor (defaults to \ref FCPP_ANTIALIAS).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::refresh_rate associates to the refresh rate (0 for opportunistic frame refreshing, defaults to \ref FCPP_REFRESH_RATE).
 * - \ref tags::threads associates to the number of threads that can be created (defaults to \ref FCPP_THREADS).
 *
 * If no color tags or color values are specified, the color defaults to white.
 */
template <class... Ts>
struct displayer {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Antialiasing factor.
    constexpr static size_t antialias = common::option_num<tags::antialias, FCPP_ANTIALIAS, Ts...>;

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

    //! @brief Base colors of nodes (defaults to white).
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
        REQUIRE_COMPONENT(displayer,identifier);
        REQUIRE_COMPONENT(displayer,positioner);
        REQUIRE_COMPONENT(displayer,storage);
        REQUIRE_COMPONENT(displayer,timer);

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
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_nbr_uids(), m_prev_nbr_uids() {}

            //! @brief Caches the current position for later use.
            glm::vec3 const& cache_position(times_t t) {
                return m_position = to_vec3(P::node::position(t));
            }

            //! @brief Accesses the cached position.
            glm::vec3 const& get_cached_position() const {
                return m_position;
            }

            //! @brief Updates the internal status of node component.
            void draw() const {
                PROFILE_COUNT("displayer");
                // gather shape and size
                shape s = common::get_or<shape_tag>(P::node::storage_tuple(), shape(shape_val));
                double d = common::get_or<size_tag>(P::node::storage_tuple(), double(size_val));
                // gather color list
                std::vector<color> c;
                color_val_push(c, color_val{});
                color_tag_push(c, color_tag{});
                if (c.empty()) c.emplace_back(1.0f, 1.0f, 1.0f, 1.0f); // white if nothing else
                // gather personal and neighbours' positions
                glm::vec3 p = get_cached_position();
                std::vector<glm::vec3> np;
                for (device_t d : m_prev_nbr_uids)
                    np.push_back(P::node::net.node_at(d).get_cached_position());
                // render the node
                P::node::net.getRenderer().drawCube(p, d, c);
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                PROFILE_COUNT("displayer");
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
        };

        //! @brief The global part of the component.
        class net : public P::net {
        public: // visible by node objects and the main program
          //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S, T>& t) :
                P::net{ t },
                m_threads( common::get_or<tags::threads>(t, FCPP_THREADS) ),
                m_refresh{ 0 },
                m_step( common::get_or<tags::refresh_rate>(t, FCPP_REFRESH_RATE) ),
                m_viewport_max{ -INF, -INF, -INF },
                m_viewport_min{ +INF, +INF, +INF },
                m_renderer{antialias},
                m_mouseLastX{ 0.0f },
                m_mouseLastY{ 0.0f },
                m_mouseRightX{ 0.0f },
                m_mouseRightY{ 0.0f },
                m_mouseFirst{ 1 },
                m_mouseRight{ 0 },
                m_deltaTime{ 0.0f },
                m_lastFrame{ 0.0f },
                m_lastPause{ false } {}

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
                    PROFILE_COUNT("displayer");
                    times_t t = P::net::internal_time();
                    auto n_beg = P::net::node_begin();
                    auto n_end = P::net::node_end();
                    if (rt == 0) {
                        common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&] (size_t i, size_t) {
                            viewport_update(n_beg[i].second.cache_position(t));
                        });
                    } else {
                        common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&] (size_t i, size_t) {
                            n_beg[i].second.cache_position(t);
                        });
                    }
                    glfwMakeContextCurrent(NULL);
                    common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&n_beg,this] (size_t i, size_t) {
                        n_beg[i].second.draw();
                    });
                    glfwMakeContextCurrent(m_renderer.getWindow());
                    if (rt == 0) {
                        // first frame only: set camera position, rotation, sensitivity
                        glm::vec3 viewport_size = m_viewport_max - m_viewport_min;
                        glm::vec3 camera_pos = (m_viewport_min + m_viewport_max) / 2.0f;
                        double dz = std::max(viewport_size.x/m_renderer.getAspectRatio(), viewport_size.y);
                        dz /= tan(m_renderer.getViewAngle() / 2) * 2;
                        camera_pos.z = m_viewport_max.z + dz;
                        // roll/pitch angles should be zero (why is there no "roll" angle in your code so far?)
                        // yaw should be so that the front of the camera is towards negative values of z (not clear to me where the zero angle starts)
                        double zFar = sqrt(dz * (dz + viewport_size.z)) * 32;
                        double zNear = zFar / 1024;
                        // zFar/zNear also regulates the cameraSensitivity to input (speed of changes)
                        // mousewheel changes zFar & zNear & cameraSensitivity (all proportionally)
                        m_renderer.setLightPosition(camera_pos);
                        m_renderer.setDefaultCameraView(camera_pos, glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
                        m_renderer.setFarPlane((float)zFar);
                        m_renderer.setNearPlane((float)zNear);
                        double diagonal = glm::length(viewport_size);
                        double grid_scale = 1;
                        while (grid_scale * 100 < diagonal) grid_scale *= 10;
                        while (grid_scale * 10 > diagonal) grid_scale /= 10;
                        m_renderer.setGridScale(grid_scale);
                        setInternalCallbacks(); // call this after m_renderer is initialized
                    }
                    /**
                     * Do whatever global management you need to do here
                     * (drawing the scene, interpreting the user input).
                     * You may want to display the current simulation time t.
                     */

                    // Draw grid
                    m_renderer.drawGrid(m_viewport_min, m_viewport_max, 0.3f);

                    // Draw orthogonal axis
                    //m_renderer.drawOrtho();
                    
                    // Draw simulation time (t)
                    m_renderer.drawText("Simulation time: " + std::to_string(t), 16.0f, 16.0f, 0.25f, glm::vec3{1.0});
                    
                    // Swap buffers and prepare for next frame to draw
                    m_renderer.swapAndNext();
                    
                    // Update deltaTime
                    updateDeltaTime();
                    
                    // Update m_refresh
                    m_refresh = rt + m_step;
                } else P::net::update();
            }

            //! @brief Returns net's Renderer object.
            fcpp::internal::Renderer const& getRenderer() {
                return m_renderer;
            }
            
            //! @brief Returns net's delta time.
            float const& getDeltaTime() {
                return m_deltaTime;
            }

        private: // implementation details
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
                float currentFrame{ (float)glfwGetTime() };
                m_deltaTime = currentFrame - m_lastFrame;
                m_lastFrame = currentFrame;
            }
        
            //! @brief It binds internally-defined callback functions to OpenGL events.
            void setInternalCallbacks() {
                // Associates this (the net instance) to m_window
                glfwSetWindowUserPointer(m_renderer.getWindow(), this);

                // Viewport callback
                glfwSetFramebufferSizeCallback(m_renderer.getWindow(), [](GLFWwindow* window, int width, int height) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    dspl.m_renderer.viewportResize(width, height);
                    });

                // Keyboard callback
                glfwSetKeyCallback(m_renderer.getWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    
                    if ( action == GLFW_PRESS ) {
                        // Check if the key is just pressed or it is already
                        std::unordered_set<int>::const_iterator found = dspl.m_key_stroked.find(key); // find key in stroked list
                        if (found == dspl.m_key_stroked.end()) {
                            dspl.m_key_stroked.insert(key); // insert key in list if not previously found
                            dspl.keyboardInput(key, true, dspl.getDeltaTime());
                        } else {
                            dspl.keyboardInput(key, false, dspl.getDeltaTime());
                        }
                    } else if ( action == GLFW_RELEASE ) {
                        dspl.m_key_stroked.erase(key);
                    }
                    
                    /*// Erase not pressed keys from stroked list
                    for ( std::unordered_set<int>::const_iterator it = myset.begin(); it != myset.end(); ++it ) {
                        
                    }*/
                });

                // Cursor position callback
                glfwSetCursorPosCallback(m_renderer.getWindow(), [](GLFWwindow* window, double xpos, double ypos) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    
                    if (dspl.m_mouseFirst) {
                        dspl.m_mouseLastX = (float)xpos;
                        dspl.m_mouseLastY = (float)ypos;
                        dspl.m_mouseFirst = false;
                    }
                    
                    float xoffset{ (float)(xpos - dspl.m_mouseLastX) };
                    float yoffset{ (float)(dspl.m_mouseLastY - ypos) }; // reversed since y-coordinates range from bottom to top
                    dspl.m_mouseLastX = (float)xpos;
                    dspl.m_mouseLastY = (float)ypos;

                    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor
                        dspl.m_renderer.mouseInput(xoffset, yoffset, 0.0, 0.0, mouse_type::fpp);
                    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // show cursor
                        if (!dspl.m_mouseRight) {
                            dspl.m_mouseRight = 1;
                            dspl.m_mouseRightX = xpos - (float)(dspl.m_renderer.getCurrentWidth() / 2);
                            dspl.m_mouseRightY = (float)(dspl.m_renderer.getCurrentHeight() / 2) - ypos;
                        }
                        dspl.m_renderer.mouseInput(xoffset, yoffset, dspl.m_mouseRightX, dspl.m_mouseRightY, mouse_type::drag); // need to move (0,0) at the center of the screen
                    }

                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
                        dspl.m_mouseRight = 0;
                        dspl.m_mouseRightX = 0.0f;
                        dspl.m_mouseRightY = 0.0f;
                    }
                    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // show cursor
                });

                // Cursor scroll callback
                glfwSetScrollCallback(m_renderer.getWindow(), [](GLFWwindow* window, double xoffset, double yoffset) {
                    net& dspl = *((net*)glfwGetWindowUserPointer(window)); // get the net instance from window
                    dspl.m_renderer.mouseInput(xoffset, yoffset, 0.0, 0.0, mouse_type::scroll);
                });
            }
            
            //! @brief Given the key stroke, the press status and a deltaTime, it manages keyboard input for the displayer and other classes.
            void keyboardInput(int key, bool first, float deltaTime) {
                // Process displayer's input
                if (key == GLFW_KEY_ESCAPE and first) {
                    glfwSetWindowShouldClose(m_renderer.getWindow(), true);
                    if (P::net::frequency() == 0) P::net::frequency(1);
                    P::net::terminate();
                }
                if (key == GLFW_KEY_I and first) {
                    // decelerate simulation
                    P::net::frequency(pow(0.5, deltaTime)*P::net::frequency());
                }
                if (key == GLFW_KEY_O and first) {
                    // decelerate simulation
                    P::net::frequency(pow(2.0, deltaTime)*P::net::frequency());
                }
                if (key == GLFW_KEY_P and first) {
                    // play/pause simulation
                    if (not m_lastPause) {
                        real_t f = P::net::frequency();
                        P::net::frequency(f == 0 ? 1 : 0);
                        m_lastPause = true;
                    }
                } else m_lastPause = false;
                
                // Process renderer's input
                m_renderer.keyboardInput(key, first, deltaTime);
            }

            //! @brief The number of threads to be used.
            const size_t m_threads;

            //! @brief The next refresh time.
            times_t m_refresh;

            //! @brief The step between refresh times.
            times_t m_step;
            
            //! @brief Net's Renderer object; it has the responsability of calling OpenGL functions.
            fcpp::internal::Renderer m_renderer;
            
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

            //! @brief Time between current frame and last frame.
            float m_deltaTime;

            //! @brief Time of last frame.
            float m_lastFrame;

            //! @brief Whether pause was pressed last time.
            bool m_lastPause;
            
            //! @brief List of currently stroked keys.
            std::unordered_set<int> m_key_stroked;

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
