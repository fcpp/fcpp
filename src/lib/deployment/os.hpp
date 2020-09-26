// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file os.hpp
 * @brief Abstract functions defining an OS interface.
 */

#ifndef FCPP_DEPLOYMENT_OS_H_
#define FCPP_DEPLOYMENT_OS_H_

#include <cassert>

#include <atomic>
#include <thread>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/mutex.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Type for raw messages received.
struct message_type {
    //! @brief Timestamp of message receival.
    times_t time;
    //! @brief UID of the sender device.
    device_t device;
    //! @brief An estimate of the signal power (RSSI).
    double power;
    //! @brief The message content.
    std::vector<char> content;
};


//! @brief Namespace containing OS-dependent functionalities.
namespace os {


//! @brief Access the local unique identifier.
device_t uid();


/**
 * @brief Low-level interface for hardware network capabilities.
 *
 * It should have the following minimal public interface:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * struct data_type;                       // default-constructible type for settings
 * data_type data;                         // network settings
 * transceiver(data_type);                 // constructor with settings
 * void send(device_t, std::vector<char>); // broadcasts a given message
 * message_type receive();                 // receives the next incoming message (empty if no incoming message)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct transceiver;


/**
 * @brief Higher-level interface for network capabilities.
 *
 * @param push Whether incoming messages should be immediately pushed to the node.
 * @param node_t The node type. It has the following method that can be called at any time:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * void receive(message_type&);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * @param transceiver_t The transceiver type.
 */
template <bool push, typename node_t, typename transceiver_t = transceiver>
class network {
  public:
    //! @brief Default-constructible type for network settings.
    using data_type = typename transceiver_t::data_type;

    //! @brief Constructor with default settings.
    network(node_t& n) : m_node(n), m_transceiver({}), m_manager(&network::manage, this) {}

    //! @brief Constructor with given settings.
    network(node_t& n, data_type d) : m_node(n), m_transceiver(d), m_manager(&network::manage, this) {}
    
    ~network() {
        m_running = false;
        m_manager.join();
    }

    //! @brief Access to network settings.
    data_type& data() {
        return m_transceiver.data;
    }

    //! @brief Const access to network settings.
    data_type const& data() const {
        return m_transceiver.data;
    }

    //! @brief Schedules the broadcast of a message.
    void send(std::vector<char> m) {
        common::lock_guard<true> l(m_mutex);
        m_send = std::move(m);
    }

    //! @brief Retrieves the collection of incoming messages.
    std::vector<message_type> receive() {
        assert(not push);
        common::lock_guard<true> l(m_mutex);
        std::vector<message_type> m = std::move(m_receive);
        m_receive.clear();
        return m;
    }

  private:
    //! @brief Manages the send and receive of messages.
    void manage() {
        while (m_running) {
            common::lock_guard<true> l(m_mutex);
            if (m_send.empty()) {
                // receiving
                message_type m = m_transceiver.receive();
                if (not m.content.empty()) {
                    if (push) {
                        common::unlock_guard<true> u(m_mutex);
                        m_node.receive(m);
                    } else m_receive.push_back(std::move(m));
                }
            } else {
                // sending
                m_transceiver.send(m_node.uid, std::move(m_send));
                m_send.clear();
            }
        }
    }

    //! @brief Reference to the node object.
    node_t& m_node;

    //! @brief Low-level hardware interface.
    transceiver_t m_transceiver;

    //! @brief Thread managing send and receive of messages.
    std::thread m_manager;

    //! @brief A mutex for regulating network operations.
    common::mutex<true> m_mutex;

    //! @brief Collection of received messages.
    std::vector<message_type> m_receive;

    //! @brief Message to be sent.
    std::vector<char> m_send;

    //! @brief Whether the object is alive and running.
    std::atomic<bool> m_running{true};
};


}


}

#endif // FCPP_DEPLOYMENT_OS_H_
