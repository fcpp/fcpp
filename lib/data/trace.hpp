// TODO: all

// typedef uint64_t trace_t: in the future might be upgraded to uint128_t
// constexpr int     k_hash_len = 48: upgrade to 104 if uint128_t
// constexpr trace_t k_hash_mod = (trace_t(1)<<k_hash_len)-1
// constexpr trace_t k_hash_factor = 15003L
// constexpr trace_t k_hash_inverse = 169676710615955L

// class trace
//  - keeps updated representation of the current stack trace
//  - works for function calls, if used as:
//      T func(trace_t __, ...) {
//          push(__);
//          ...
//          pop()
//      }
//      ... func(___, ...) ...
//  - works for cycles, if used as:
//      push_cycle(___);
//      someway_repeating { push(___); ....; }
//      pop_cycle();
//  - for branching, we use "delayed alignment" through:
//      ... ? align(___, ...) : align(___, ...)
//      ... = align(___, ...)

// members:
//  - vector of trace_t (used as a stack)
//  - trace_t summarising hash (48 bits used, starting from 0)

// methods:
//  - hash(trace_t x): returns the hash together with the argument into a 64 bit integer
//    return h + (x << k_hash_len)

//  - push(trace_t): add function call id to stack trace updating the hash
//    h = (h * k_hash_factor + x) & k_hash_mod

//  - pop(): remove last function call from stack trace updating the hash
//    h = ((h + k_hash_mod+1 - x) * k_hash_inverse) & k_hash_mod

//  - push_cycle(trace_t x): calls push with x + k_hash_mod+1

//  - pop_cycle(): calls pop until removing value larger than k_hash_mod
