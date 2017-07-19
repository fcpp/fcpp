// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

// TODO: doxygen

// class exports:
//  - heterogeneous data indexed by stack-trace hash
//  - data added by message-exchanging constructs
//  - exports are supposed to be exchanged at end of rounds

// parameters:
//  - Ts...: types included in the export

// members:
//  - tuple<map<trace_t, Ts>...> data
//  - set<trace_t> points (for void-type data)

// methods for the value map:
//  - insert(hash, v): inserts value at corresponding hash
//  - count(hash): whether the hash is present in the value map or not
//  - at(hash): const-reference to the value at the given hash

// methods for the code-points set:
//  - insert(hash): inserts hash into points set
//  - contains(hash): whether the hash is in the points set


#ifndef FCPP_DATATYPE_EXPORTS_H_
#define FCPP_DATATYPE_EXPORTS_H_

#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "lib/data/trace.hpp"
#include "lib/util/traits.hpp"


namespace fcpp {


template <typename... Ts>
class exports {
    static_assert(not type_repeated<Ts...>, "cannot instantiate exports with repeated types");
    
    std::tuple<std::unordered_map<trace_t, Ts>...> data;
    std::unordered_set<trace_t> points;
    
public:
    exports() = default;
    
    exports(const exports<Ts...>&) = default;
    
    exports(exports<Ts...>&&) = default;
    
    exports<Ts...>& operator=(const exports<Ts...>&) = default;
    
    exports<Ts...>& operator=(exports<Ts...>&&) = default;
    
    bool operator==(const exports<Ts...>& o) const {
        return points == o.points && data == o.data;
    }
    
    template<typename A>
    void insert(trace_t trace, const A& value) {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data)[trace] = value;
    }

    template<typename A>
    bool count(trace_t trace) const {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        return std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data).count(trace);
    }

    template<typename A>
    const A& at(trace_t trace) const {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        return std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data).at(trace);
    }
    
    void insert(trace_t trace) {
        points.insert(trace);
    }
    
    bool contains(trace_t trace) {
        return points.count(trace);
    }
};


}

#endif  // FCPP_DATATYPE_EXPORT_H_
