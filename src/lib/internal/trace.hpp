// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file trace.hpp
 * @brief Implementation of the `trace` class for identifying code points.
 */

#ifndef FCPP_INTERNAL_TRACE_H_
#define FCPP_INTERNAL_TRACE_H_

//! @brief Macro for uniquely identifying source code locations.
#define ___ __COUNTER__

#include <cassert>
#include <cstdint>

#include <vector>
#include <functional>

#include "lib/settings.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


#if   FCPP_TRACE == 16
    typedef uint16_t trace_t;
    constexpr int k_hash_len = 12;
    constexpr trace_t k_hash_factor = 17;
    constexpr trace_t k_hash_inverse = 241;
#elif FCPP_TRACE == 24
    typedef uint32_t trace_t;
    constexpr int k_hash_len = 16;
    constexpr trace_t k_hash_factor = 17;
    constexpr trace_t k_hash_inverse = 61681;
#elif FCPP_TRACE == 32
    typedef uint32_t trace_t;
    constexpr int k_hash_len = 20;
    constexpr trace_t k_hash_factor = 33;
    constexpr trace_t k_hash_inverse = 1016801;
#elif FCPP_TRACE == 48
    typedef uint64_t trace_t;
    constexpr int k_hash_len = 34;
    constexpr trace_t k_hash_factor = 3251L;
    constexpr trace_t k_hash_inverse = 10500276859L;
#elif FCPP_TRACE == 64
    typedef uint64_t trace_t;
    constexpr int k_hash_len = 48;
    constexpr trace_t k_hash_factor = 4871L;
    constexpr trace_t k_hash_inverse = 33111303973559L;
#else
    static_assert(false, "invalid value for FCPP_TRACE");
    //! @brief Type for trace hashes (depends on @ref FCPP_TRACE).
    typedef uint32_t trace_t;
    //! @brief Bit size of a trace hash.
    constexpr int k_hash_len = 0;
    //! @brief The factor by which the hash is multiplied when a new item enters the trace.
    constexpr trace_t k_hash_factor = 0;
    //! @brief The inverse of the factor modulo the hash length.
    constexpr trace_t k_hash_inverse = 0;
#endif

//! @brief Value for quickly computing the reduction to @ref k_hash_len bits.
constexpr trace_t k_hash_mod = (trace_t(1)<<k_hash_len)-1;

//! @brief Maximium value allowed for code counters.
constexpr trace_t k_hash_max = (trace_t(1)<<(FCPP_TRACE - k_hash_len))-1;


//! @brief Namespace containing objects of internal use.
namespace internal {


//! @cond INTERNAL
//! @brief Forward declarations for friendship.
struct trace_reset;
struct trace_call;
struct trace_key;
struct trace_cycle;
//! @endcond


/**
 * @brief Keeps an updated representation of the current stack trace.
 *
 * Should be used only indirectly through classes @ref trace_call and @ref trace_cycle.
 * In order to handle branching, we follow "delayed alignment" by inserting `align`
 * calls into conditional operators and assignments.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * ... ? align(___, ...) : align(___, ...)
 * ... = align(___, ...)
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class trace {
    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    friend struct trace_reset;
    friend struct trace_call;
    friend struct trace_key;
    friend struct trace_cycle;
    //! @}
    //! @endcond

  public:
    //! @brief Constructs an empty trace.
    trace() : m_stack{}, m_stack_hash{0} {};

    //! @brief `true` if the trace is empty, `false` otherwise.
    bool empty() const {
        return m_stack.size() == 0;
    }

    //! @brief Returns the hash together with the template argument into a @ref trace_t.
    inline trace_t hash(trace_t x) const {
        assert((x <= k_hash_max or !FCPP_WARNING_TRACE) and "code points overflow: reduce code or increase FCPP_TRACE (ignore with #define FCPP_WARNING_TRACE false if using few CALLs for each function)");
        return m_stack_hash + ((x & k_hash_max) << k_hash_len);
    }

  protected:
    //! @brief Clears the trace.
    void clear() {
        m_stack_hash = 0;
        m_stack.clear();
    }

    //! @brief Add a function call to the stack trace updating the hash.
    inline void push(trace_t x) {
        assert(x <= k_hash_mod and "code points overflow: reduce code or increase FCPP_TRACE");
        assert((x < k_hash_factor or !FCPP_WARNING_TRACE) and "warning: code points may induce colliding hashes (ignore with #define FCPP_WARNING_TRACE false)");
        m_stack_hash = (m_stack_hash * k_hash_factor + x) & k_hash_mod;
        m_stack.push_back(x);
    }

    //! @brief Adds a custom hashed key to the stack trace updating the hash.
    inline void push_key(trace_t x) {
        x &= k_hash_mod;
        m_stack_hash = (m_stack_hash * k_hash_factor + x) & k_hash_mod;
        m_stack.push_back(x);
    }

    //! @brief Remove the last function call from the stack trace updating the hash.
    inline void pop() {
        trace_t x = m_stack.back();
        m_stack.pop_back();
        m_stack_hash = ((m_stack_hash + k_hash_mod+1 - x) * k_hash_inverse) & k_hash_mod;
    }

  private:
    //! @brief Stack trace.
    std::vector<trace_t> m_stack;
    //! @brief Summarising hash (@ref k_hash_len bits used, starting from 0).
    trace_t m_stack_hash;
};


//! @brief Stateless class ensuring execution within an empty trace.
struct trace_reset {
    //! @brief Constructor (clears the trace).
    trace_reset(trace& t) : m_trace{t} {
        m_trace.clear();
    }
    //! @brief Destructor (clears the trace).
    ~trace_reset() {
        m_trace.clear();
    }

  private:
    //! @brief Reference trace object.
    trace& m_trace;
};


/**
 * @brief Stateless class for handling trace update on function call.
 *
 * The intended usage is:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * template <class node_t, ...>
 * type func(node_t& node, trace_t call_point, ...) {
 *     internal::trace_call trace_caller(node.stack_trace, call_point);
 *     ...
 * }
 * ... func(node, __COUNTER__, ...) ...
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct trace_call {
    //! @brief Constructor (adds element to trace).
    trace_call(trace& t, trace_t x) : m_trace{t} {
        m_trace.push(x);
    }
    //! @brief Destructor (removes element from trace).
    ~trace_call() {
        m_trace.pop();
    }

  private:
    //! @brief Reference trace object.
    trace& m_trace;
};

/**
 * @brief Stateless class for handling trace update on process keys.
 *
 * The intended usage is:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * internal::trace_key trace_process(node.stack_trace, key);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct trace_key {
    //! @brief Constructor (adds element to trace).
    trace_key(trace& t, trace_t x) : m_trace{t} {
        m_trace.push_key(x);
    }
    //! @brief Destructor (removes element from trace).
    ~trace_key() {
        m_trace.pop();
    }

  private:
    //! @brief Reference trace object.
    trace& m_trace;
};

/**
 * @brief Stateless class for handling trace update on cycles.
 *
 * The intended usage is:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * {
 *     internal::trace_cycle trace_cycler(node.stack_trace);
 *     while (...) {
 *         ....
 *         ++trace_cycler;
 *     }
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Additionally, a `trace_cycle` can be directly used as a `trace_t` index:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * {
 *     for (internal::trace_cycle i{node.stack_trace, 1}; i < N; ++i) {
 *         ... // can use i as trace_t index here
 *     }
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct trace_cycle {
    //! @brief Constructor (adds a starting cycle element to the trace).
    trace_cycle(trace& t, trace_t i = 0) : m_trace{t}, m_i{i} {
        m_trace.push(m_i);
    }
    //! @brief Destructor (removes the cycle element from the trace).
    ~trace_cycle() {
        m_trace.pop();
    }
    //! @brief Increment operator (increases the cycle element in the trace).
    inline trace_cycle& operator++() {
        m_trace.pop();
        m_trace.push(++m_i);
        return *this;
    }
    //! @brief Decrement operator (decreases the cycle element in the trace).
    inline trace_cycle& operator--() {
        m_trace.pop();
        m_trace.push(--m_i);
        return *this;
    }
    //! @brief Increasing operator (increases the cycle element in the trace).
    inline trace_cycle& operator+=(trace_t x) {
        m_trace.pop();
        m_trace.push(m_i+=x);
        return *this;
    }
    //! @brief Decreasing operator (decreases the cycle element in the trace).
    inline trace_cycle& operator-=(trace_t x) {
        m_trace.pop();
        m_trace.push(m_i-=x);
        return *this;
    }
    //! @brief Returns the current cycle element.
    inline operator trace_t() {
        return m_i;
    }

  private:
    //! @brief Reference trace object.
    trace& m_trace;

    //! @brief Cycle index.
    trace_t m_i;
};


}


}

#endif // FCPP_INTERNAL_TRACE_H_
