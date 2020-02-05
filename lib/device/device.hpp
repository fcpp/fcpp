// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file device.hpp
 * @brief Implementation of the `device<M, Ts...>` class template for grouping message data from different neighbours.
 */

#ifndef FCPP_DEVICE_DEVICE_H_
#define FCPP_DEVICE_DEVICE_H_

#include <functional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "lib/common/multitype_map.hpp"
#include "lib/common/traits.hpp"
#include "lib/common/twin.hpp"
#include "lib/data/context.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Abstraction of a FCPP device.
 *
 * Provides context and implementation for the FCPP basic constructs,
 * together with methods for handling insertion and removal of old
 * (or less relevant) exports.
 *
 * Direct subclasses of the `device` are provided in FCPP, adding sensors,
 * actuators and constructors depending on the intended deployment platform.
 * Virtual subclasses are also available as libraries of FCPP functions.
 * User-defined subclasses should extend the given ones with user-specific
 * functions and the `round()` function to be executed periodically.
 *
 * Broadcasting of messages, actual sensing/actuation, scheduling of events
 * are not part of this class and are managed by a separate simulator or
 * handler component.
 *
 * The \p M class should:
 * - be a variadic template with class arguments:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <class... Ts> class M { ... };
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - provide a `result_type` type member which has to be totally ordered, for example:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   typedef double result_type;
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - the following type member is also suggested:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   typedef multitype_map<trace_t, Ts...> export_type;
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - provide a static `metric` method, computing the distance between two exports,
 *   with the following signature:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   static result_type metric(const export_type& self, const export_type& other) {
 *       ...
 *   }
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 *   This method is called to compute the "goodness" of an incoming message with
 *   respect to the result of the previous round on the current device.
 *
 * Valid instances of the \p M class are provided in the FCPP distribution: TODO.
 *
 * @param M  Callable class realising a metric on exports.
 * @param Ts Types included in the exports.
 */
template <template<class...> class M, class... Ts>
class device {
  public:
    //! @brief The type of the metric on exports.
    typedef typename M<Ts...>::result_type metric_type;
    
    //! @brief The type of the exports of other devices.
    typedef typename context<metric_type, Ts...>::export_type context_type;
    
    //! @brief The type of the exports of the current device (`first` is for the local device, `second` for other devices).
    typedef twin<multitype_map<trace_t, Ts...>, FCPP_SETTING_EXPORTS == 1> export_type;

  protected:
    /** @brief Stateless class for handling trace update on function call.
     *
     * The intended usage is:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * template<trace_t __>
     * T func(...) {
     *     trace_call<__> _;
     *     ...
     * }
     * ... func<___>(...) ...
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template<trace_t x>
    struct trace_call {
        //! @brief Constructor (adds element to trace).
        trace_call() {
            thread_trace.push<x>();
        }
        //! @brief Destructor (removes element from trace).
        ~trace_call() {
            thread_trace.pop();
        }
    };
    
    /** @brief Stateless class for handling trace update on cycles.
     *
     * The intended usage is:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * {
     *     trace_cycle<___> _;
     *     someway_repeating {
     *         _();
     *         ....
     *     }
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template<trace_t x>
    struct trace_cycle {
        //! @brief Constructor (adds a cycle element to trace).
        trace_cycle() {
            thread_trace.push_cycle<x>();
        }
        //! @brief Destructor (removes all cycle elements from trace).
        ~trace_cycle() {
            thread_trace.pop_cycle();
        }
        //! @brief Call operator (adds a further element to trace).
        void operator()() {
            thread_trace.push<x>();
        }
    };

  private:
    //! @brief Map associating devices to their exports.
    context<metric_type, Ts...> m_context;
    //! @brief Exports of the current device.
    export_type m_export;
    //! @brief Maximum amount of neighbours allowed
    device_t m_neighbours;
    //! @brief Maximum export metric value allowed
    metric_type m_metric;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    device(device_t self, device_t neighbours, metric_type metric) : m_context(self), m_export(), m_neighbours(neighbours), m_metric(metric) {};
    
    //! @brief Copy constructor.
    device(const device<M, Ts...>&) = default;
    
    //! @brief Move constructor.
    device(device<M, Ts...>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    device<M, Ts...>& operator=(const device<M, Ts...>&) = default;
    
    //! @brief Move assignment.
    device<M, Ts...>& operator=(device<M, Ts...>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const device<M, Ts...>& o) const {
        return m_context == o.m_context && m_export == o.m_export && m_neighbours == o.m_neighbours && m_metric == o.m_metric;
    }

    //! @brief Access to the local device identifier.
    device_t self() const {
        return m_context.self();
    }
    
    //! @brief Number of neighbours (including self).
    size_t size() const {
        return m_context.size();
    }
    
    //! @brief Inserts an export for a device.
    void insert(device_t device, context_type e) {
        m_context.insert(device, std::move(e), M<Ts...>::metric(m_export.second(), *e));
        if (m_context.size() > m_neighbours) m_context.pop();
    }
    
    //! @brief Sets up the initial value of the export data (from sensors).
    void round_start(export_type e) {
        m_export = std::move(e);
    }

    //! @brief Recomputes metrics, cleaning obsolete values (to be run at end of rounds).
    void round_end() {
        for (const auto& x : m_context.data())
            m_context.insert(x.first, M<Ts...>::metric(m_export.second(), *(x.second)));
        while (m_context.top() > m_metric) m_context.pop();
    }
    
  protected:
    //! @name field operators
    //@{
    //! @brief Selects the local value of a field.
    template <typename A>
    del_template<field, A> self(A&& x) const {
        return details::self(std::forward<A>(x), m_context.self());
    }
    
    //! @brief Write access to the default value of a field, ensuring alignment.
    template<typename A>
    A& other(field<A>& f) const {
        return details::other(align(f));
    }

    //! @brief Computes the restriction of a field to a given domain.
    template <trace_t x, typename A>
    A align(A&& f) const {
        trace_t t = thread_trace.hash<x>();
        m_export.second().insert(t);
        return details::align(std::forward<A>(f), m_context.align(t));
    }
    
    //! @brief Reduces the values in the domain of a field to a single value through a binary operation.
    template <trace_t x, typename F, typename A>
    local_result<F,A,A> fold_hood(F&& op, const A& f) const {
        trace_t t = thread_trace.hash<x>();
        m_export.second().insert(t);
        return details::fold_hood(op, f, m_context.align(t));
    }
    //@}

    //! @name old-based coordination operators
    //@{
    /**
     * @brief The previous-round value of the argument.
     *
     * Equivalent to `old(f, f)`.
     */
    template <trace_t x, typename A>
    const A& old(const A& f) {
        trace_t t = thread_trace.hash<x>();
        m_export.first().insert(t, f);
        return m_context.old(t, f);
    }
    /**
     * @brief The previous-round value of the second argument, defaulting to the first argument if no previous value.
     *
     * Equivalent to:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * old(f0, [](const A& fo){
     *     return std::make_pair(fo, f);
     * })
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template <trace_t x, typename A>
    const A& old(const A& f0, const A& f) {
        trace_t t = thread_trace.hash<x>();
        m_export.first().insert(t, f);
        return m_context.old(t, f0);
    }
    /**
     * @brief The previous-round value of the result (defaults to first argument), modified through the second argument.
     *
     * Applies if the \p op argument has return type `A`.
     * Corresponds to the `rep` construct of the field calculus. Equivalent to:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * old(f0, [](const A& fo){
     *     A f = op(fo);
     *     return std::make_pair(f, f);
     * })
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template <trace_t x, typename A>
    A old(const A& f0, std::function<A(const A&)> op) {
        trace_t t = thread_trace.hash<x>();
        A f = op(m_context.old(t, f0));
        m_export.first().insert(t, f);
        return std::move(f);
    }
    /**
     * @brief The previous-round value (defaults to first argument), modified through the second argument.
     *
     * Applies if the \p op argument has return type `std::pair<B,A>`.
     * The first element of the returned pair is returned by the function.
     * The second element of the returned pair is written in the exports.
     */
    template <trace_t x, typename A, typename B>
    B old(const A& f0, std::function<std::pair<B,A>(const A&)> op) {
        trace_t t = thread_trace.hash<x>();
        std::pair<B,A> f = op(m_context.old(t, f0));
        m_export.first().insert(t, std::move(f.second));
        return std::move(f.first);
    }
    //@}

    //! @name nbr-based coordination operators
    //@{
    /**
     * @brief The neighbours' value of the argument.
     *
     * Equivalent to `nbr(f, f)`.
     */
    template <trace_t x, typename A>
    add_template<field, A> nbr(const A& f) {
        trace_t t = thread_trace.hash<x>();
        m_export.second().insert(t, f);
        return m_context.nbr(t, f);
    }
    /**
     * @brief The neighbours' value of the second argument, defaulting to the first argument.
     *
     * Equivalent to:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * nbr(f0, [](add_template<field, A> fn){
     *     return std::make_pair(fn, f);
     * })
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template <trace_t x, typename A>
    add_template<field, A> nbr(const A& f0, const A& f) {
        trace_t t = thread_trace.hash<x>();
        m_export.second().insert(t, f);
        return m_context.nbr(t, f0);
    }
    /**
     * @brief The neighbours' value of the result (defaults to first argument), modified through the second argument.
     *
     * Applies if the \p op argument has return type `A`.
     * Equivalent to:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * nbr(f0, [](add_template<field, A> fn){
     *     A f = op(fn);
     *     return std::make_pair(f, f);
     * })
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template <trace_t x, typename A>
    A nbr(const A& f0, std::function<A(add_template<field, A>)> op) {
        trace_t t = thread_trace.hash<x>();
        A f = op(m_context.nbr(t, f0));
        m_export.second().insert(t, f);
        return std::move(f);
    }
    /**
     * @brief The neighbours' value (defaults to first argument), modified through the second argument.
     *
     * Applies if the \p op argument has return type `std::pair<B,A>`.
     * The first element of the returned pair is returned by the function.
     * The second element of the returned pair is written in the exports.
     */
    template <trace_t x, typename A, typename B>
    B nbr(const A& f0, std::function<std::pair<B,A>(add_template<field, A>)> op) {
        trace_t t = thread_trace.hash<x>();
        std::pair<B,A> f = op(m_context.nbr(t, f0));
        m_export.second().insert(t, std::move(f.second));
        return std::move(f.first);
    }
    //@}

    //! @name mixed coordination operators
    //@{
    /**
     * @brief The result of the second argument given info from neighbours' and self.
     *
     * Applies if the \p op argument has return type `A`.
     * Equivalent to:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * share(f0, [](const A& fo, add_template<field, A> fn){
     *     A f = op(fo, fn);
     *     return std::make_pair(f, f);
     * })
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     */
    template <trace_t x, typename A>
    A oldnbr(const A& f0, std::function<A(const A&, add_template<field, A>)> op) {
        trace_t t = thread_trace.hash<x>();
        A f = op(m_context.old(t, f0), m_context.nbr(t, f0));
        m_export.second().insert(t, f);
        return std::move(f);
    }
    /**
     * @brief The result of the second argument given info from neighbours' and self.
     *
     * Applies if the \p op argument has return type `std::pair<B,A>`.
     * The first element of the returned pair is returned by the function.
     * The second element of the returned pair is written in the exports.
     */
    template <trace_t x, typename A, typename B>
    B oldnbr(const A& f0, std::function<std::pair<B,A>(const A&, add_template<field, A>)> op) {
        trace_t t = thread_trace.hash<x>();
        std::pair<B,A> f = op(m_context.old(t, f0), m_context.nbr(t, f0));
        m_export.second().insert(t, std::move(f.second));
        return std::move(f.first);
    }
    //@}
};


}

#endif // FCPP_DEVICE_DEVICE_H_
