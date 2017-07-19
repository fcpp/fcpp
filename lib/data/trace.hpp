// Copyright © 2017 Giorgio Audrito and Roberto Poletti. All Rights Reserved.

#ifndef FCPP_DATA_TRACE_H_
#define FCPP_DATA_TRACE_H_

#include <cstdint>
#include <vector>

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief in the future might be upgraded to uint128_t
typedef uint64_t trace_t;

//! @brief upgrade to 104 if uint128_t
constexpr int k_hash_len = 48;

constexpr trace_t k_hash_mod = (trace_t(1)<<k_hash_len)-1;

constexpr trace_t k_hash_factor = 15003L;

constexpr trace_t k_hash_inverse = 169676710615955L;


/** @brief keeps updated representation of the current stack trace
 *  - works for function calls, if used as:
 *      T func(trace_t __, ...) {
 *         push(__);
 *         ...
 *          pop()
 *      }
 *      ... func(___, ...) ...
 *  - works for cycles, if used as:
 *      push_cycle(___);
 *      someway_repeating { push(___); ....; }
 *      pop_cycle();
 *  - for branching, we use "delayed alignment" through:
 *      ... ? align(___, ...) : align(___, ...)
 *      ... = align(___, ...)
 */
class trace {
  private:
	//! @brief vector of trace_t (used as a stack)
	std::vector<trace_t> stack;
	//! @ brief trace_t summarising hash (48 bits used, starting from 0)
    trace_t stack_hash;

  public:
    trace() : stack(), stack_hash(0) {};

    //! @brief returns the hash together with the argument into a 64 bit integer
    trace_t hash(trace_t x) {
    	return stack_hash + (x << k_hash_len);
    }

    //! @brief add function call id to stack trace updating the hash
    void push(trace_t x) {
    	stack_hash = (stack_hash * k_hash_factor + x) & k_hash_mod;
    	stack.push_back(x);
    }

    //! @brief remove last function call from stack trace updating the hash
    void pop() {
    	trace_t x = stack.back();
    	stack.pop_back();
    	stack_hash = ((stack_hash + k_hash_mod+1 - x) * k_hash_inverse) & k_hash_mod;
    }

    //! @brief calls push with x + k_hash_mod+1
    void push_cycle(trace_t x) {
    	push(x + k_hash_mod+1);
    }

    //! @brief calls pop until removing value larger than k_hash_mod
    void pop_cycle() {
    	while (stack.back() < k_hash_mod) pop();
    	pop();
    }
};


}


#endif
