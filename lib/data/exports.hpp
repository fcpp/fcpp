// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

// TODO: rename into multitype_map and generalize key type

/**
 * @file exports.hpp
 * @brief Implementation of the exports<Ts...> class template for handling message data.
 */

#ifndef FCPP_DATATYPE_EXPORTS_H_
#define FCPP_DATATYPE_EXPORTS_H_

#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "lib/data/trace.hpp"
#include "lib/util/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Class for handling heterogeneous data.
 *
 * Data is indexed by stack-trace hash and added by message-exchanging constructs.
 * Exports are exchanged between devices at the end of rounds.
 *
 * @param Ts Types to include in the export (must avoid repetitions).
 */
template <typename... Ts>
class exports {
    static_assert(not type_repeated<Ts...>, "cannot instantiate exports with repeated types");
    
    //! @brief Map associating hashes to data.
    std::tuple<std::unordered_map<trace_t, Ts>...> data;
    //! @brief Set of hashes (for void data).
    std::unordered_set<trace_t> points;
    
public:
    //! @name constructors
    //@{
    /**
     * @brief Default constructor (creates an empty structure).
     */
    exports() = default;
    
    //! @brief Copy constructor.
    exports(const exports<Ts...>&) = default;
    
    //! @brief Move constructor.
    exports(exports<Ts...>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    exports<Ts...>& operator=(const exports<Ts...>&) = default;
    
    //! @brief Move assignment.
    exports<Ts...>& operator=(exports<Ts...>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const exports<Ts...>& o) const {
        return points == o.points && data == o.data;
    }
    
    //! @brief Inserts value at corresponding hash.
    template<typename A>
    void insert(trace_t trace, const A& value) {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data)[trace] = value;
    }

    //! @brief Whether the hash is present in the value map or not.
    template<typename A>
    bool count(trace_t trace) const {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        return std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data).count(trace);
    }

    //! @brief Immutable reference to the value at a given hash.
    template<typename A>
    const A& at(trace_t trace) const {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        return std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data).at(trace);
    }
    
    //! @brief Inserts hash into points set.
    void insert(trace_t trace) {
        points.insert(trace);
    }
    
    //! @brief Whether the hash is in the points set.
    bool contains(trace_t trace) {
        return points.count(trace);
    }
};


}

#endif  // FCPP_DATATYPE_EXPORT_H_
