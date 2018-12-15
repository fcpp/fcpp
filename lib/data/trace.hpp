// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file trace.hpp
 * @brief Implementation of the `trace` class for identifying code points.
 */

#ifndef FCPP_DATA_TRACE_H_
#define FCPP_DATA_TRACE_H_

//! @brief Macro for uniquely identifying source code locations.
#define ___ __COUNTER__

#include <cstdint>
#include <vector>

#include "lib/settings.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


#if   FCPP_SETTING_TRACE == 24
    typedef uint32_t trace_t;
    constexpr int k_hash_len = 16;
    constexpr trace_t k_hash_factor = 17;
    constexpr trace_t k_hash_inverse = 61681;
#elif FCPP_SETTING_TRACE == 32
    typedef uint32_t trace_t;
    constexpr int k_hash_len = 20;
    constexpr trace_t k_hash_factor = 33;
    constexpr trace_t k_hash_inverse = 1016801;
#elif FCPP_SETTING_TRACE == 48
    typedef uint64_t trace_t;
    constexpr int k_hash_len = 34;
    constexpr trace_t k_hash_factor = 3251L;
    constexpr trace_t k_hash_inverse = 10500276859L;
#elif FCPP_SETTING_TRACE == 64
    typedef uint64_t trace_t;
    constexpr int k_hash_len = 48;
    constexpr trace_t k_hash_factor = 4871L;
    constexpr trace_t k_hash_inverse = 33111303973559L;
#else
    static_assert(false, "invalid value for FCPP_SETTING_TRACE");
    //! @brief Type for trace hashes (depends on @ref FCPP_SETTING_TRACE).
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
constexpr trace_t k_hash_max = trace_t(1)<<(FCPP_SETTING_TRACE - k_hash_len);

    
/** @brief Keeps an updated representation of the current stack trace.
 *  The intended usage is:
 *  - for function definition and call,
 *    ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *    template<trace_t __>
 *    T func(...) {
 *        push<__>();
 *        ...
 *        pop();
 *    }
 *    ... func<___>(...) ...
 *    ~~~~~~~~~~~~~~~~~~~~~~~~~
 *  - for cycles,
 *    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *    push_cycle<___>();
 *    someway_repeating {
 *        push<___>();
 *        ....
 *    }
 *    pop_cycle();
 *    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  - to handle branching, we follow "delayed alignment" by inserting `align`
 *    calls into conditional operators and assignments.
 *    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *    ... ? align<___>(...) : align<___>(...)
 *    ... = align<___>(...)
 *    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class trace {
  private:
    //! @brief Stack trace.
    std::vector<trace_t> stack;
    //! @brief Summarising hash (@ref k_hash_len bits used, starting from 0).
    trace_t stack_hash;

  public:
    //! @brief Constructs an empty trace.
    trace() : stack(), stack_hash(0) {};

    //! @brief `true` if the trace is empty, `false` otherwise.
    bool empty() const {
        return stack.size() == 0;
    }
    
    //! @brief Clears the trace.
    void clear() {
        stack_hash = 0;
        stack.clear();
    }
    
    //! @brief Returns the hash together with the template argument into a @ref trace_t.
    template<trace_t x>
    trace_t hash() {
        static_assert(x < k_hash_max, "code points overflow: reduce code or increase FCPP_SETTING_TRACE");
    	return stack_hash + (x << k_hash_len);
    }

    //! @brief Add a function call to the stack trace updating the hash.
    template<trace_t x>
    void push() {
        static_assert(x < k_hash_max, "code points overflow: reduce code or increase FCPP_SETTING_TRACE");
    	stack_hash = (stack_hash * k_hash_factor + x) & k_hash_mod;
    	stack.push_back(x);
    }

    //! @brief Remove the last function call from the stack trace updating the hash.
    void pop() {
    	trace_t x = stack.back();
    	stack.pop_back();
    	stack_hash = ((stack_hash + k_hash_mod+1 - x) * k_hash_inverse) & k_hash_mod;
    }

    //! @brief Calls push with `x + k_hash_mod+1`.
    template<trace_t x>
    void push_cycle() {
        static_assert(x < k_hash_max, "code points overflow: reduce code or increase FCPP_SETTING_TRACE");
    	push<x>();
        stack.back() += k_hash_mod+1;
    }

    //! @brief Calls pop until removing value larger than @ref k_hash_mod.
    void pop_cycle() {
    	while (stack.back() <= k_hash_mod) pop();
        stack.back() -= k_hash_mod+1;
    	pop();
    }
};

    
//! @brief A global trace variable, maintained separately for different threads.
thread_local trace thread_trace;


}

#endif // FCPP_DATA_TRACE_H_
