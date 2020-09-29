// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file fake_os.hpp
 * @brief Concrete functions defining a fake OS interface.
 */

#ifndef FCPP_FAKE_OS_H_
#define FCPP_FAKE_OS_H_

#include "lib/common/mutex.hpp"
#include "lib/deployment/os.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing OS-dependent functionalities.
namespace os {


//! @brief Access the local unique identifier.
device_t uid() {
    return 42;
}


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
struct transceiver {
    using data_type = transceiver*;

    data_type data;

    transceiver(data_type) : data(this) {}

    void send(device_t, std::vector<char> m) {
        assert(not sending and not receiving);
        sending = true;
        common::lock_guard<true> l(m_mutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        m_out = std::move(m);
        sending = false;
    }

    message_type receive() {
        assert(not sending and not receiving);
        receiving = true;
        common::lock_guard<true> l(m_mutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        message_type m;
        if (not m_in.empty()) {
            m = std::move(m_in.back());
            m_in.pop_back();
        }
        else std::this_thread::sleep_for(std::chrono::milliseconds(10));
        receiving = false;
        return m;
    }

    std::vector<char> fake_send() {
        common::lock_guard<true> l(m_mutex);
        std::vector<char> v;
        std::swap(v, m_out);
        return v;
    }

    void fake_receive(message_type m) {
        common::lock_guard<true> l(m_mutex);
        m_in.push_back(m);
    }

  private:
    common::mutex<true> m_mutex;

    std::vector<message_type> m_in;

    std::vector<char> m_out;

    bool sending = false;

    bool receiving = false;
};


}


}

#endif // FCPP_FAKE_OS_H_
