// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file plot.hpp
 * @brief Plot building tools.
 */

#ifndef FCPP_COMMON_PLOT_H_
#define FCPP_COMMON_PLOT_H_

#include <cstdint>
#include <ctime>

#include <limits>
#include <map>
#include <ratio>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

#include "lib/common/mutex.hpp"
#include "lib/common/serialize.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/option/aggregator.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for plot building tools.
namespace plot {

//! @cond INTERNAL
namespace details {
    //! @brief Shortens a string.
    std::string shorten(std::string s);

    //! @brief Shortens a title string.
    std::string multi_shorten(std::string s);
}
//! @endcond

//! @brief Tag for time information.
struct time {};


//! @brief Structure representing a point in a plot.
struct point {
    //! @brief Default constructor.
    point() = default;
    //! @brief The unit of the measured value.
    std::string unit;
    //! @brief The source of the measured value.
    std::string source;
    //! @brief The measured value.
    double value;
    //! @brief The upwards deviation of the value.
    double devup;
    //! @brief The downwards deviation of the value.
    double devdn;
    //! @brief The default value used when the deviation is not available.
    static constexpr double no_dev = -std::numeric_limits<double>::infinity();
    //! @brief Whether the plot should be considered an empty plot.
    bool empty() const {
        return true;
    }
};

//! @brief Printing a single point.
template <typename O>
O& operator<<(O& o, point const& p) {
    o << "(";
    if (p.unit.size()) o << p.unit << ", ";
    o << p.source << ", " << p.value;
    if (p.devup != point::no_dev or p.devdn != point::no_dev) o << " + " << p.devup << " - " << p.devdn;
    o << ")";
    return o;
}


//! @brief Structure representing a single plot.
struct plot {
    //! @brief Default constructor.
    plot() = default;
    //! @brief Title of the plot.
    std::string title;
    //! @brief Label of the x axis.
    std::string xname;
    //! @brief Label of the y axis.
    std::string yname;
    //! @brief Values of x coordinates.
    std::vector<double> xvals;
    //! @brief Values of y coordinates with labels.
    std::vector<std::pair<std::string, std::vector<std::tuple<double,double,double>>>> yvals;
    //! @brief Whether the plot should be considered an empty plot.
    bool empty() const {
        return xvals.empty();
    }
};

//! @brief Printing a single plot.
template <typename O>
O& operator<<(O& o, plot const& p) {
    if (p.empty()) return o;
    o << "plot.put(plot.plot(name+\"-" << details::shorten(p.xname) << details::shorten(p.yname) << (p.title.size() ? "-" : "") << details::multi_shorten(p.title) << "\", \"" << p.title << "\", \"" << p.xname << "\", \"" << p.yname << "\", new string[] {";
    bool first = true;
    for (auto const& y : p.yvals) {
        if (first) first = false;
        else o << ", ";
        o << "\"" << y.first << "\"";
    }
    o << "}, new pair[][] {";
    first = true;
    for (auto const& y : p.yvals) {
        if (first) first = false;
        else o << ", ";
        o << "{";
        for (size_t i=0; i<y.second.size(); ++i) {
            if (i > 0) o << ", ";
            o << "(" << p.xvals[i] << ", " << std::get<0>(y.second[i]) << ")";
        }
        o << "}";
    }
    bool hasdev = false;
    for (auto const& y : p.yvals) for (size_t i=0; i<y.second.size() and not hasdev; ++i)
        if (std::get<1>(y.second[i]) != point::no_dev or std::get<2>(y.second[i]) != point::no_dev)
            hasdev = true;
    if (hasdev) {
        o << "}, new real[][] {";
        first = true;
        for (auto const& y : p.yvals) {
            if (first) first = false;
            else o << ", ";
            o << "{";
            for (size_t i=0; i<y.second.size(); ++i) {
                if (i > 0) o << ", ";
                o << std::get<1>(y.second[i]);
            }
            o << "}";
        }
        hasdev = false;
        for (auto const& y : p.yvals) for (size_t i=0; i<y.second.size() and not hasdev; ++i)
            if (std::get<2>(y.second[i]) != point::no_dev)
                hasdev = true;
        if (hasdev) {
            o << "}, new real[][] {";
            first = true;
            for (auto const& y : p.yvals) {
                if (first) first = false;
                else o << ", ";
                o << "{";
                for (size_t i=0; i<y.second.size(); ++i) {
                    if (i > 0) o << ", ";
                    o << std::get<2>(y.second[i]);
                }
                o << "}";
            }
        }
    }
    o << "}));\n";
    return o;
}


//! @brief Structure representing a page of plots.
struct page {
    //! @brief Default constructor.
    page() = default;
    //! @brief Constructor with a vector of plots.
    page(std::vector<plot> p) : title(), rows(1), cols(p.size()), plots(p) {}
    //! @brief Constructor with an array of plots.
    template <size_t N>
    page(std::array<plot, N> p) : title(), rows(1), cols(N), plots(p.begin(), p.end()) {}
    //! @brief Title of the page.
    std::string title;
    //! @brief Number of rows.
    size_t rows;
    //! @brief Number of columns.
    size_t cols;
    //! @brief Plot list.
    std::vector<plot> plots;
    //! @brief Whether the plot should be considered an empty plot.
    bool empty() const {
        for (auto const& p : plots) if (not p.empty()) return false;
        return true;
    }
};

//! @brief Printing a single page.
template <typename O>
O& operator<<(O& o, page const& p) {
    if (p.empty()) return o;
    if (p.title.size()) o << "// " << p.title << "\n\n";
    o << "plot.ROWS = " << p.rows << ";\n";
    o << "plot.COLS = " << p.cols << ";\n\n";
    for (plot const& q : p.plots) o << q << "\n";
    return o;
}


//! @brief Structure representing a whole file of plots.
struct file {
    //! @brief The type for options.
    using option_type = std::vector<std::pair<std::string, std::string>>;
    //! @brief Constructor with a vector of pages.
    file(std::string title, std::vector<page> p, option_type opt = {}) : title(title), pages(p), options(opt) {}
    //! @brief Constructor with an array of pages.
    template <size_t N>
    file(std::string title, std::array<page, N> p, option_type opt = {}) : title(title), pages(p.begin(), p.end()), options(opt) {}
    //! @brief Constructor with a vector of plots.
    file(std::string title, std::vector<plot> p, option_type opt = {}) : title(title), options(opt) {
        pages.emplace_back(p);
    }
    //! @brief Constructor with an array of plots.
    template <size_t N>
    file(std::string title, std::array<plot, N> p, option_type opt = {}) : title(title), options(opt) {
        pages.emplace_back(p);
    }
    //! @brief Title of the file.
    std::string title;
    //! @brief Page list.
    std::vector<page> pages;
    //! @brief Custom plot options.
    option_type options;
    //! @brief Whether the plot should be considered an empty plot.
    bool empty() const {
        for (auto const& p : pages) if (not p.empty()) return false;
        return true;
    }
};

//! @brief Printing a file.
template <typename O>
O& operator<<(O& o, file const& f) {
    if (f.empty()) return o;
    o << "// " << f.title << "\n";
    o << "string name = \"" << f.title << "\";\n\n";
    o << "import \"plot.asy\" as plot;\n";
    o << "unitsize(1cm);\n\n";
    if (not f.options.empty()) {
        for (auto const& k : f.options) o << "plot." << k.first << " = " << k.second << ";\n";
        o << "\n";
    }
    for (page const& p : f.pages) o << p << "\n";
    o << "shipout(\"" << f.title << "\");\n";
    return o;
}


//! @brief Empty class not producing a plot.
class none {
  public:
    //! @brief The internal build type.
    using build_type = typename std::array<point, 0>;

    //! @brief Default constructor.
    none() = default;

    //! @brief Copy constructor.
    none(none const&) = default;

    //! @brief Move constructor.
    none(none&&) = default;

    //! @brief Copy assignment.
    none& operator=(none const&) = default;

    //! @brief Move assignment.
    none& operator=(none&&) = default;

    //! @brief Row processing.
    template <typename R>
    none& operator<<(R const&) {
        return *this;
    }

    //! @brief Equality operator.
    bool operator==(none const& o) const {
        return true;
    }

    //! @brief Inequality operator.
    bool operator!=(none const& o) const {
        return !(*this == o);
    }

    //! @brief Plot merging.
    none& operator+=(none const&) {
        return *this;
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        return {};
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) const {
        return s;
    }
};


//! @cond INTERNAL
namespace details {
    //! @brief Extract any multi-type option from a single type.
    template <typename T>
    struct option_types {
        using type = common::type_sequence<>;
    };
    template <template<class...> class T, typename... Ts>
    struct option_types<T<Ts...>> {
        using type = common::type_sequence<Ts...>;
    };

    //! @brief Integer holding a bit for every type in a type sequence.
    template <typename T>
    struct integer_delta_type;
    template <size_t n>
    struct integer_delta_type<std::array<int, n>> {
        using type = std::conditional_t<n <= 16,
            std::conditional_t<n <= 8,  uint8_t,  uint16_t>,
            std::conditional_t<n <= 32, uint32_t, uint64_t>
        >;
    };
    template <typename... Ts>
    struct integer_delta_type<common::type_sequence<Ts...>> {
        using type = typename integer_delta_type<std::array<int, sizeof...(Ts)>>::type;
    };

    //! @brief Tagged tuple wrapper, serialising the difference with a reference tuple.
    template <typename T>
    class delta_tuple : public T {
        //! @brief Type for storing deltas.
        using delta_type = typename integer_delta_type<typename T::tags>::type;

      public:
        //! @brief Default constructor.
        delta_tuple() : T(), m_ref(*this) {}

        //! @brief Reference constructor.
        delta_tuple(delta_tuple const& t) : T(), m_ref(t) {}

        //! @brief Inherit assignment operators.
        using T::operator=;

        //! @brief Assignment with delta tuples.
        delta_tuple& operator=(delta_tuple const& t) {
            T::operator=(static_cast<T const&>(t));
            return *this;
        }

        //! @brief Serialises the content from a given input stream.
        common::isstream& serialize(common::isstream& s) {
            delta_type d = serialize_delta(s);
            serialize_impl(s, d, typename T::tags{});
            return s;
        }

        //! @brief Serialises the content to a given output stream.
        common::osstream& serialize(common::osstream& s) const {
            delta_type d = serialize_delta(s);
            serialize_impl(s, d, typename T::tags{});
            return s;
        }

      private:
        //! @brief Serialises the delta bits.
        delta_type serialize_delta(common::isstream& s) {
            delta_type d;
            s.read(d);
            return d;
        }
        delta_type serialize_delta(common::type_sequence<>) const {
            return 0;
        }
        template <typename S, typename... Ss>
        delta_type serialize_delta(common::type_sequence<S, Ss...>) const {
            return (serialize_delta(common::type_sequence<Ss...>{}) << 1) + (common::get<S>(*this) == common::get<S>(m_ref));
        }
        delta_type serialize_delta(common::osstream& s) const {
            delta_type d = serialize_delta(typename T::tags{});
            s.write(d);
            return d;
        }

        //! @brief Serialises a skipped field.
        template <typename S>
        inline void serialize_skip(common::isstream const&, common::type_sequence<S>) {
            common::get<S>(*this) = common::get<S>(m_ref);
        }
        template <typename S>
        inline void serialize_skip(common::osstream const&, common::type_sequence<S>) const {}

        //! @brief Serialises given delta and tags.
        template <typename S>
        void serialize_impl(S&, delta_type, common::type_sequence<>) const {}
        template <typename S, typename S1, typename... Ss>
        void serialize_impl(S& s, delta_type d, common::type_sequence<S1, Ss...>) {
            if (d & 1) serialize_skip(s, common::type_sequence<S1>{});
            else s & common::get<S1>(*this);
            serialize_impl(s, d >> 1, common::type_sequence<Ss...>{});
        }
        template <typename S1, typename... Ss>
        void serialize_impl(common::osstream& s, delta_type d, common::type_sequence<S1, Ss...>) const {
            if (d & 1) serialize_skip(s, common::type_sequence<S1>{});
            else s << common::get<S1>(*this);
            serialize_impl(s, d >> 1, common::type_sequence<Ss...>{});
        }

        //! @brief Reference tuple.
        delta_tuple const& m_ref;
    };
}
//! @endcond

/**
 * @brief Plotter storing all rows for later printing.
 *
 * @tparam C Tags and types to be delta-compressed upon serialisation.
 * @tparam M Tags and types not compressed on serialisation.
 * @tparam F Tags and types assumed constant and stored only once.
 * @tparam max_size The maximum size in bytes allowed for the buffer (0 for no maximum size).
 */
template <typename C, typename M = void, typename F = void, size_t max_size = 0>
class rows {
  public:
    //! @brief Sequence of tags and types to be delta-compressed upon serialisation.
    using compressible_tuple_type = details::delta_tuple<common::tagged_tuple_t<typename details::option_types<C>::type>>;
    //! @brief Sequence of tags and types not compressed on serialisation.
    using mutable_tuple_type = common::tagged_tuple_t<typename details::option_types<M>::type>;
    //! @brief Sequence of tags and types assumed constant and stored only once.
    using fixed_tuple_type = common::tagged_tuple_t<typename details::option_types<F>::type>;

    //! @brief The expected limit size of the object.
    static constexpr size_t limit_size = max_size;

    //! @brief Default constructor.
    rows() {
        m_start = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        m_rows.data().reserve(max_size);
        m_length = m_row_size = 0;
    }

    //! @brief Row processing.
    template <typename R>
    rows& operator<<(R const& row) {
        size_t prev_size = m_rows.size();
        if (max_size == 0 or prev_size + m_row_size < max_size) {
            m_fixed = row;
            compressible_tuple_type tc(m_last);
            mutable_tuple_type tm;
            tc = row;
            tm = row;
            (m_rows << tc << tm);
            m_last = row;
            m_row_size = std::max(m_row_size, m_rows.size() - prev_size);
            ++m_length;
        }
        return *this;
    }

    //! @brief The number of rows stored.
    size_t size() const {
        return m_length;
    }

    //! @brief The number of bytes occupied by the structure.
    size_t byte_size() const {
        return sizeof(rows) + m_rows.size();
    }

    //! @brief Prints the object's contents.
    template <typename O>
    void print(O& o) {
        using std::swap;
        common::isstream rows({});
        swap(rows.data(), m_rows.data());
        std::string tstr = std::string(ctime(&m_start));
        tstr.pop_back();
        o << "########################################################\n";
        o << "# FCPP execution started at:  " << tstr << " #\n";
        o << "########################################################\n# ";
        m_fixed.print(o, common::assignment_tuple);
        o << "\n#\n";
        o << "# The columns have the following meaning:\n# ";
        print_headers(o, typename mutable_tuple_type::tags{});
        print_headers(o, typename compressible_tuple_type::tags{});
        o << "\n";
        m_last = compressible_tuple_type();
        for (size_t i=0; i<m_length; ++i) {
            compressible_tuple_type tc(m_last);
            mutable_tuple_type tm;
            rows >> tc >> tm;
            m_last = tc;
            print_output(o, tm, typename mutable_tuple_type::tags{});
            print_output(o, tc, typename compressible_tuple_type::tags{});
            o << "\n";
        }
        std::time_t m_end = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        tstr = std::string(ctime(&m_end));
        tstr.pop_back();
        o << "########################################################\n";
        o << "# FCPP execution finished at: " << tstr << " #\n";
        o << "########################################################" << std::endl;
        swap(rows.data(), m_rows.data());
    }

  private:
    //! @brief Prints the storage headers.
    template <typename O>
    void print_headers(O&, common::type_sequence<>) const {}
    template <typename O, typename U, typename... Us>
    void print_headers(O& o, common::type_sequence<U,Us...>) const {
        o << common::strip_namespaces(common::type_name<U>()) << " ";
        print_headers(o, common::type_sequence<Us...>{});
    }

    //! @brief Prints the storage values.
    template <typename O, typename T>
    void print_output(O&, T const&, common::type_sequence<>) const {}
    template <typename O, typename T, typename U, typename... Us>
    void print_output(O& o, T const& t, common::type_sequence<U,Us...>) const {
        o << common::escape(common::get<U>(t)) << " ";
        print_output(o, t, common::type_sequence<Us...>{});
    }

    //! @brief Start time.
    std::time_t m_start;
    //! @brief Fixed data.
    fixed_tuple_type m_fixed;
    //! @brief Last row of compressible data.
    compressible_tuple_type m_last;
    //! @brief Serialised rows.
    common::osstream m_rows;
    //! @brief Number of rows stored.
    size_t m_length;
    //! @brief Maximum size of a row ever recorded.
    size_t m_row_size;
};

template <typename C, typename M, typename F, size_t max_size>
constexpr size_t rows<C,M,F,max_size>::limit_size;


//! @brief Filters values for columns and properties in a plotter.
template <typename... Ts>
class filter {
    //! @brief The type sequence of type parameters.
    using types = common::type_sequence<Ts...>;
    //! @brief The plotter type.
    using plotter_type = typename types::back;
    //! @brief The tuple of filters.
    using filter_type = common::tagged_tuple_t<typename types::pop_back>;

  public:
    //! @brief The internal build type.
    using build_type = typename plotter_type::build_type;

    //! @brief Default constructor.
    filter() = default;

    //! @brief Copy constructor.
    filter(filter const&) = default;

    //! @brief Move constructor.
    filter(filter&&) = default;

    //! @brief Copy assignment.
    filter& operator=(filter const&) = default;

    //! @brief Move assignment.
    filter& operator=(filter&&) = default;

    //! @brief Row processing.
    template <typename R>
    filter& operator<<(R const& row) {
        if (check(row, typename filter_type::tags{}))
            m_plotter << row;
        return *this;
    }

    //! @brief Plot merging.
    filter& operator+=(filter const& o) {
        m_plotter += o.m_plotter;
        return *this;
    }

    //! @brief Equality operator.
    bool operator==(filter const& o) const {
        return m_plotter == o.m_plotter;
    }

    //! @brief Inequality operator.
    bool operator!=(filter const& o) const {
        return !(*this == o);
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        return m_plotter.build();
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename T>
    T& serialize(T& s) {
        return s & m_plotter;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename T>
    T& serialize(T& s) const {
        return s << m_plotter;
    }

  private:
    //! @brief Checks whether the row complies to the filters (empty overload).
    template <typename R>
    inline bool check(R const&, common::type_sequence<>) {
        return true;
    }
    //! @brief Checks whether the row complies to the filters (active overload).
    template <typename R, typename S, typename... Ss>
    inline bool check(R const& row, common::type_sequence<S, Ss...>) {
        return common::get<S>(m_filters)(common::get<S>(row)) and check(row, common::type_sequence<Ss...>{});
    }

    //! @brief The plotter.
    plotter_type m_plotter;
    //! @brief The callable filter class.
    filter_type m_filters;
};


//! @cond INTERNAL
namespace details {
    //! @brief Concatenate vector and array types.
    template <typename... Ts>
    struct array_cat;
    //! @brief Concatenate one vector type.
    template <typename T>
    struct array_cat<std::vector<T>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate one array type.
    template <typename T, size_t N>
    struct array_cat<std::array<T,N>> {
        using type = std::array<T,N>;
    };
    //! @brief Concatenate two compatible vector types.
    template <typename T>
    struct array_cat<std::vector<T>, std::vector<T>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate vector and array types.
    template <typename T, size_t N>
    struct array_cat<std::vector<T>, std::array<T,N>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate array and vector types.
    template <typename T, size_t N>
    struct array_cat<std::array<T,N>, std::vector<T>> {
        using type = std::vector<T>;
    };
    //! @brief Concatenate two compatible array types.
    template <typename T, size_t N, size_t M>
    struct array_cat<std::array<T,N>, std::array<T,M>> {
        using type = std::array<T,N+M>;
    };
    //! @brief Concatenate plots and pages arrays.
    template <size_t N, size_t M>
    struct array_cat<std::array<plot,N>, std::array<page,M>> {
        using type = std::array<page,M+1>;
    };
    template <size_t N, size_t M>
    struct array_cat<std::array<page,N>, std::array<plot,M>> {
        using type = std::array<page,N+1>;
    };
    //! @brief Concatenate plots and pages vectors.
    template <>
    struct array_cat<std::vector<plot>, std::vector<page>> {
        using type = std::vector<page>;
    };
    template <>
    struct array_cat<std::vector<page>, std::vector<plot>> {
        using type = std::vector<page>;
    };
    //! @brief Concatenate plots arrays and pages vectors.
    template <size_t N>
    struct array_cat<std::array<plot,N>, std::vector<page>> {
        using type = std::vector<page>;
    };
    template <size_t N>
    struct array_cat<std::vector<page>, std::array<plot,N>> {
        using type = std::vector<page>;
    };
    //! @brief Concatenate plots vectors and pages arrays.
    template <size_t N>
    struct array_cat<std::vector<plot>, std::array<page,N>> {
        using type = std::array<page,N+1>;
    };
    template <size_t N>
    struct array_cat<std::array<page,N>, std::vector<plot>> {
        using type = std::array<page,N+1>;
    };
    //! @brief Concatenate multiple vector and array types.
    template <typename T, typename U, typename... Us>
    struct array_cat<T, U, Us...> {
        using type = typename array_cat<T, typename array_cat<U, Us...>::type>::type;
    };

    //! @brief Resizes a vector.
    template <typename T>
    void maybe_resize(std::vector<T>& v, size_t s) {
        v.resize(s);
    }
    //! @brief Does not resize an array.
    template <typename T, size_t N>
    void maybe_resize(std::array<T,N>&, size_t) {}

    //! @brief Does not convert a vector.
    template <typename T>
    inline std::vector<T> maybe_promote(common::type_sequence<T>, std::vector<T> v) {
        return v;
    }
    //! @brief Does not convert an array.
    template <typename T, size_t N>
    inline std::array<T,N> maybe_promote(common::type_sequence<T>, std::array<T,N> v) {
        return v;
    }
    //! @brief Converts a vector of plots.
    inline std::array<page,1> maybe_promote(common::type_sequence<page>, std::vector<plot> v) {
        return {v};
    }
    //! @brief Converts an array of plots.
    template <size_t N>
    inline std::array<page,1> maybe_promote(common::type_sequence<page>, std::array<plot,N> v) {
        return {v};
    }
}
//! @endcond

//! @brief Joins a sequence of plotters.
template <typename... Ps>
class join {
  public:
    //! @brief The internal build type.
    using build_type = typename details::array_cat<typename Ps::build_type...>::type;

    //! @brief Default constructor.
    join() = default;

    //! @brief Row processing.
    template <typename R>
    join& operator<<(R const& row) {
        input_impl(row, std::make_index_sequence<sizeof...(Ps)>{});
        return *this;
    }

    //! @brief Copy constructor.
    join(join const&) = default;

    //! @brief Move constructor.
    join(join&&) = default;

    //! @brief Copy assignment.
    join& operator=(join const&) = default;

    //! @brief Move assignment.
    join& operator=(join&&) = default;

    //! @brief Plot merging.
    join& operator+=(join const& o) {
        sum_impl(o, std::make_index_sequence<sizeof...(Ps)>{});
        return *this;
    }

    //! @brief Equality operator.
    bool operator==(join const& o) const {
        return m_plotters == o.m_plotters;
    }

    //! @brief Inequality operator.
    bool operator!=(join const& o) const {
        return !(*this == o);
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        build_type res;
        build_impl(res, std::make_index_sequence<sizeof...(Ps)>{});
        return res;
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_plotters;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_plotters;
    }

  private:
    //! @brief Forwarding rows to plotters.
    template <typename R, size_t... is>
    void input_impl(R const& row, std::index_sequence<is...>) {
        common::ignore_args((std::get<is>(m_plotters) << row)...);
    }

    //! @brief Stop joining plotter builds.
    void build_impl(build_type&, std::index_sequence<>, size_t) const {}

    //! @brief Joining plotter builds.
    template <size_t i, size_t... is>
    void build_impl(build_type& res, std::index_sequence<i, is...>, size_t x = 0) const {
        auto ri = details::maybe_promote(common::type_sequence<typename build_type::value_type>{}, std::get<i>(m_plotters).build());
        details::maybe_resize(res, x+ri.size());
        for (size_t j = 0; j < ri.size(); ++j)
            res[x+j] = std::move(ri[j]);
        build_impl(res, std::index_sequence<is...>{}, x + ri.size());
    }

    //! @brief Merges all sub-plots.
    template <size_t... is>
    inline void sum_impl(join const& o, std::index_sequence<is...>) {
        common::ignore_args((std::get<is>(m_plotters) += std::get<is>(o.m_plotters))...);
    }

    //! @brief The plotters.
    std::tuple<Ps...> m_plotters;
};

//! @brief Optimised Join of a single plotter.
template <typename P>
class join<P> : public P {};


//! @cond INTERNAL
namespace details {
    //! @brief Formats a type description.
    inline std::string& format_type(std::string& s) {
        for (size_t i=0; i<s.size(); ++i) if (s[i] == '_') s[i] = ' ';
        return s;
    }
    //! @brief Formats a type description.
    inline std::string format_type(std::string&& s) {
        return format_type(s);
    }
    //! @brief Formats a type description.
    inline std::string format_type(std::string const& s) {
        return format_type(std::string(s));
    }

    //! @brief Checks whether C has a static name method.
    template<typename C>
    struct has_name_method {
      private:
        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(T::name()),
            std::string
        >::type;

        template <typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = type::value;
    };
}
//! @endcond

//! @brief Maintains a value for the column S aggregated with A.
template <typename S, typename A = aggregator::only_finite<aggregator::mean<double>>>
class value {
  public:
    //! @brief The internal build type.
    using build_type = std::array<point, 1>;

    //! @brief Default constructor.
    value() = default;

    //! @brief Row processing.
    template <typename R>
    value& operator<<(R const& row) {
        common::lock_guard<true> l(m_mutex);
        m_aggregator.insert(common::get<S>(row));
        return *this;
    }

    //! @brief Copy constructor.
    value(value const& o) : m_aggregator(o.m_aggregator) {}

    //! @brief Move constructor.
    value(value&& o) : m_aggregator(o.m_aggregator) {}

    //! @brief Copy assignment.
    value& operator=(value const& o) {
        m_aggregator = o.m_aggregator;
        return *this;
    }

    //! @brief Move assignment.
    value& operator=(value&& o) {
        m_aggregator = o.m_aggregator;
        return *this;
    }

    //! @brief Plot merging.
    value& operator+=(value const& o) {
        m_aggregator += o.m_aggregator;
        return *this;
    }

    //! @brief Equality operator.
    bool operator==(value const& o) const {
        return m_aggregator == o.m_aggregator;
    }

    //! @brief Inequality operator.
    bool operator!=(value const& o) const {
        return !(*this == o);
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        point p;
        std::string t = tag_name(common::number_sequence<details::has_name_method<S>::value>{}); // tag name
        size_t pos = t.find("<");
        if (pos != std::string::npos) {
            p.unit = common::strip_namespaces(t.substr(0, pos));
            p.source = common::strip_namespaces(t.substr(pos+1, t.size()-pos-2));
        } else {
            pos = t.find("__");
            if (pos != std::string::npos) {
                p.source = common::strip_namespaces(t.substr(0, pos));
                p.unit = t.substr(pos+2);
            } else {
                p.source = common::strip_namespaces(t);
                p.unit = p.source;
            }
        }
        details::format_type(p.unit);
        details::format_type(p.source);
        std::string ar = A::name(); // row aggregator
        std::string ad = aggregator_name(common::number_sequence<details::has_name_method<S>::value>{}); // device aggregator
        p.source += " (" + ad + ar + ")";
        auto r = m_aggregator.template result<S>();
        p.value = std::get<0>(r);
        p.devup = maybe_tuple_get<1>(r);
        p.devdn = maybe_tuple_get<2>(r);
        return {p};
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename T>
    T& serialize(T& s) {
        return s & m_aggregator;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename T>
    T& serialize(T& s) const {
        return s << m_aggregator;
    }

  private:
    //! @brief Device aggregator name (if present).
    std::string aggregator_name(common::number_sequence<true>) const {
        return S::name() + "-";
    }
    //! @brief Device aggregator name (if absent).
    std::string aggregator_name(common::number_sequence<false>) const {
        return "";
    }

    //! @brief Tag name (if aggregator present).
    std::string tag_name(common::number_sequence<true>) const {
        return common::type_name<typename S::tag>();
    }
    //! @brief Tag name (if aggregator absent).
    std::string tag_name(common::number_sequence<false>) const {
        return common::type_name<S>();
    }

    //! @brief Gets the i-th element of a result tuple (if present).
    template <size_t i, typename T, typename... Ts>
    static std::enable_if_t<i < sizeof...(Ts), double>
    maybe_tuple_get(common::tagged_tuple<T, common::type_sequence<Ts...>> const& t) {
        return std::get<i>(t);
    }

    //! @brief Gets the i-th element of a result tuple (if absent).
    template <size_t i, typename T, typename... Ts>
    static std::enable_if_t<i >= sizeof...(Ts), double>
    maybe_tuple_get(common::tagged_tuple<T, common::type_sequence<Ts...>> const&) {
        return point::no_dev;
    }

    //! @brief A mutex for synchronised access to the aggregator.
    common::mutex<true> m_mutex;
    //! @brief The aggregator.
    A m_aggregator;
};


//! @brief Tag for declaring units to be extracted.
template <template<class> class... Ts>
struct unit;

//! @cond INTERNAL
namespace details {
    //! @brief Smart append of a plotter to a join.
    template <typename Q, typename... Ps>
    struct appender {
        using type = join<Ps..., Q>;
    };
    //! @brief Smart append of a join to a join.
    template <typename... Qs, typename... Ps>
    struct appender<join<Qs...>, Ps...> {
        using type = join<Ps..., Qs...>;
    };

    //! @brief Smart join of plotters.
    template <typename... Ps>
    struct joiner;
    //! @brief Smart join of one plotter.
    template <typename P>
    struct joiner<P> {
        using type = P;
    };
    //! @brief Smart join of two plotters (first non-join).
    template <typename P, typename Q>
    struct joiner<P, Q> : public appender<Q, P> {};
    //! @brief Smart join of two plotter (first join).
    template <typename... Ps, typename Q>
    struct joiner<join<Ps...>, Q> : public appender<Q, Ps...> {};
    //! @brief Smart join of multiple plotters.
    template <typename P, typename Q, typename... Qs>
    struct joiner<P, Q, Qs...>  : public joiner<typename joiner<P, Q>::type, Qs...> {};

    //! @brief Searches type T within tags and aggregators in S (general form).
    template <typename T, typename S>
    struct field_grep;
    //! @brief Searches type T within tags and aggregators in S.
    template <typename T, typename... Ss, typename... As>
    struct field_grep<T, common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<As...>>> {
        using type = common::type_cat<std::conditional_t<std::is_same<Ss,T>::value or std::is_same<As,T>::value, typename As::template result_type<Ss>::tags, common::type_sequence<>>...>;
    };

    //! @brief Searches template T within tags in S (general form).
    template <template<class> class T, typename S>
    struct unit_grep {
        using type = common::type_sequence<bool>;
    };
    //! @brief Searches template T within tags in S.
    template <template<class> class T, typename... Ss, typename... As>
    struct unit_grep<T, common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<As...>>> {
        using type = common::type_cat<std::conditional_t<common::is_class_template<T,Ss>, typename As::template result_type<Ss>::tags, common::type_sequence<>>...>;
    };

    //! @brief Maintains values for multiple columns and aggregators (general form).
    template <typename S, typename A, typename... Ts>
    struct values;
    //! @brief Maintains values for one explicit column and one aggregator.
    template <template<class...> class S, typename S1, template<class...> class A, typename A1>
    struct values<S<S1>, A<A1>> {
       using type = value<S1, A1>;
    };
    //! @brief Maintains values for multiple explicit columns and one aggregator.
    template <template<class...> class S, typename S1, typename... Ss, template<class...> class A, typename A1>
    struct values<S<S1, Ss...>, A<A1>> {
       using type = join<value<S1, A1>, value<Ss, A1>...>;
    };
    //! @brief Maintains values for multiple explicit columns and no aggregators (defaults to `mean<double>`).
    template <typename S, template<class...> class A>
    struct values<S, A<>> : public values<S, A<aggregator::only_finite<aggregator::mean<double>>>> {};
    //! @brief Maintains values for multiple explicit columns and multiple aggregators.
    template <typename S, template<class...> class A, typename A1, typename... As>
    struct values<S, A<A1, As...>> : public joiner<typename values<S, A<A1>>::type, typename values<S, A<As>>::type...> {};
    //! @brief Maintains values for multiple columns and aggregators, defined through a single field.
    template <template<class...> class S, typename... Ss, typename A, typename T>
    struct values<S<Ss...>, A, T> : public values<typename field_grep<T, common::tagged_tuple_t<common::type_sequence<Ss...>>>::type, A> {};
    //! @brief Maintains values for multiple columns and aggregators, defined through a single unit.
    template <template<class...> class S, typename... Ss, typename A, template<class> class T>
    struct values<S<Ss...>, A, unit<T>> : public values<typename unit_grep<T, common::tagged_tuple_t<common::type_sequence<Ss...>>>::type, A> {};
    //! @brief Maintains values for multiple columns and aggregators, defined through multiple units.
    template <typename S, typename A, template<class> class T, template<class> class... Ts>
    struct values<S, A, unit<T, Ts...>> : public joiner<typename values<S, A, unit<T>>::type, typename values<S, A, unit<Ts>>::type...> {};
    //! @brief Maintains values for multiple columns and aggregators, defined through multiple fields.
    template <typename S, typename A, typename T1, typename T2, typename... Ts>
    struct values<S, A, T1, T2, Ts...> : public joiner<typename values<S, A, T1>::type, typename values<S, A, T2>::type, typename values<S, A, Ts>::type...> {};
}
//! @endcond

/**
 * @brief Maintains values for multiple columns and aggregators.
 *
 * @tparam S The sequence of tags and aggregators for logging (intertwined).
 * @tparam A The sequence of row aggregators (if empty, `mean<double>` is assumed).
 * @tparam Ts Description of fields to be extracted as tags, aggregators or units (if empty, S is interpreted as fields).
 */
template <typename S, typename A, typename... Ts>
using values = typename details::values<S, A, Ts...>::type;


//! @cond INTERNAL
namespace details {
    //! @brief The type to be used for a single key.
    template <typename S>
    struct key_type {
        using type = common::tagged_tuple_t<S, double>;
        static constexpr bool single = true;
    };
    //! @brief The type to be used for multiple keys.
    template <typename... Ss>
    struct key_type<common::type_sequence<Ss...>> {
        using type = common::tagged_tuple_cat<common::tagged_tuple_t<Ss, double>...>;
        static constexpr bool single = (sizeof...(Ss) == 1);
    };

    //! @brief Inspects a build type.
    template <typename T>
    struct inspector;
    //! @brief Whether a vector build type contains a single object.
    template <typename T>
    struct inspector<std::vector<T>> {
        using type = T;
        static constexpr bool single = false;
    };
    //! @brief Whether an array build type contains a single object.
    template <typename T, size_t N>
    struct inspector<std::array<T,N>> {
        using type = T;
        static constexpr bool single = (N == 1);
    };

    //! @brief Promotes a maximal build type after split.
    template <typename P, bool single>
    struct promote_impl {
        using type = std::vector<page>;
    };
    //! @brief Promotes multiple plots after split.
    template <>
    struct promote_impl<plot, false> {
        using type = std::array<page, 1>;
    };
    //! @brief Promotes a single plot after split.
    template <>
    struct promote_impl<plot, true> {
        using type = std::vector<plot>;
    };
    //! @brief Promotes points after split.
    template <bool single>
    struct promote_impl<point, single> {
        using type = std::array<plot, 1>;
    };

    //! @brief Promotes a build type after split.
    template <typename P>
    using promote = typename promote_impl<typename inspector<P>::type, inspector<P>::single>::type;
}
//! @endcond

/**
 * Split rows depending on
 *
 * @tparam S A column tag, or a `type_sequence` of column tags.
 * @tparam P The plotter to be split.
 * @tparam B A bucket size (as `std::ratio`, only for a single key).
 */
template <typename S, typename P, typename B = std::ratio<0>>
class split {
    //! @brief The build type of dependent plotters.
    using parent_build_type = typename P::build_type;

    static_assert(details::key_type<S>::single or not std::is_same<typename details::inspector<parent_build_type>::type, point>::value, "cannot split points with multiple keys");

    //! @brief The type used for splitting keys.
    using key_type = typename details::key_type<S>::type;

  public:
    //! @brief The internal build type.
    using build_type = details::promote<parent_build_type>;

    //! @brief Default constructor.
    split() = default;

    //! @brief Copy constructor.
    split(split const& o) : m_plotters(o.m_plotters) {}

    //! @brief Move constructor.
    split(split&& o) : m_plotters(o.m_plotters) {}

    //! @brief Copy assignment.
    split& operator=(split const& o) {
        m_plotters = o.m_plotters;
        return *this;
    }

    //! @brief Move assignment.
    split& operator=(split&& o) {
        m_plotters = o.m_plotters;
        return *this;
    }

    //! @brief Row processing.
    template <typename R>
    split& operator<<(R const& row) {
        static_assert(common::type_intersect<typename R::tags, typename key_type::tags>::size == key_type::tags::size, "splitting key not in plot row");
        key_type k = approx_impl(row, B{}, typename key_type::tags{});
        P* p;
        {
            common::lock_guard<true> l(m_mutex);
            p = &m_plotters[k];
        }
        *p << row;
        return *this;
    }

    //! @brief Plot merging.
    split& operator+=(split const& o) {
        std::vector<std::pair<const key_type, P>> new_keys;
        auto it = o.m_plotters.begin();
        for (auto& x : m_plotters) {
            for (; it != o.m_plotters.end() and it->first < x.first; ++it)
                new_keys.push_back(*it);
            if (it == o.m_plotters.end()) break;
            if (it->first == x.first) x.second += it->second;
        }
        for (; it != o.m_plotters.end(); ++it)
            new_keys.push_back(*it);
        m_plotters.insert(new_keys.begin(), new_keys.end());
        return *this;
    }

    //! @brief Equality operator.
    bool operator==(split const& o) const {
        return m_plotters == o.m_plotters;
    }

    //! @brief Inequality operator.
    bool operator!=(split const& o) const {
        return !(*this == o);
    }

    //! @brief Plot building for internal use.
    build_type build() const {
        std::map<key_type, parent_build_type> m;
        for (auto const& x : m_plotters)
            m.emplace(x.first, x.second.build());
        build_type res;
        build_impl(res, m);
        return res;
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename T>
    T& serialize(T& s) {
        return s & m_plotters;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename T>
    T& serialize(T& s) const {
        return s << m_plotters;
    }

  private:
    //! @brief Approximates a single value according to a given scale.
    template <intmax_t num, intmax_t den, typename T>
    T& approx_impl(T& x) const {
        intmax_t n = x * den / num + 0.5;
        return x = double(n*num)/den;
    }

    //! @brief No approximations without buckets.
    template <typename R, typename T>
    key_type approx_impl(R const& row, std::ratio<0>, T) const {
        return row;
    }

    //! @brief Approximating the keys to their closest bucket multiple.
    template <typename R, intmax_t num, intmax_t den, typename = std::enable_if_t<num != 0>, typename... Ss>
    key_type approx_impl(R const& row, std::ratio<num,den>, common::type_sequence<Ss...>) const {
        key_type k = row;
        common::ignore_args(approx_impl<num,den>(common::get<Ss>(k))...);
        return k;
    }

    //! @brief Single plot building.
    void build_impl(std::array<plot, 1>& res, std::map<key_type, parent_build_type>& m) const {
        res[0].xname = details::format_type(common::strip_namespaces(common::type_name<S>()));
        std::set<std::string> units;
        if (m.size()) for (point const& q : m.begin()->second) {
            if (q.unit.size()) units.insert(q.unit);
            res[0].yvals.emplace_back(q.source, std::vector<std::tuple<double,double,double>>{});
        }
        if (units.empty()) res[0].yname = "y";
        else if (units.size() == 1) res[0].yname = *units.begin();
        else {
            std::vector<std::string> w1;
            std::vector<std::set<std::string>> words;
            auto filler = [](auto&& inserter, std::string const& s){
                size_t pos = 0;
                for (size_t p = s.find(" ", pos); p != std::string::npos; p = s.find(" ", pos)) {
                    inserter(s.substr(pos, p-pos));
                    pos = p+1;
                }
                inserter(s.substr(pos));
            };
            filler([&](std::string s){ w1.push_back(s); }, *units.begin());
            for (auto it = ++units.begin(); it != units.end(); ++it) {
                words.emplace_back();
                filler([&](std::string s){ words.back().insert(s); }, *it);
            }
            for (std::string const& s : w1) {
                bool ok = true;
                for (size_t i=0; ok and i<words.size(); ++i) if (words[i].count(s) == 0) ok = false;
                if (ok) res[0].yname += (res[0].yname.size() ? " " : "") + s;
            }
            if (res[0].yname.empty()) res[0].yname = "y";
        }
        if (res[0].yname.size() > 6) res[0].yname = details::shorten(res[0].yname);
        for (auto const& p : m) {
            res[0].xvals.push_back(std::get<0>(p.first));
            for (size_t i = 0; i < p.second.size(); ++i)
                res[0].yvals[i].second.emplace_back(p.second[i].value, p.second[i].devup, p.second[i].devdn);
        }
    }
    //! @brief Multiple plot building.
    void build_impl(std::vector<plot>& res, std::map<key_type, parent_build_type>& m) const {
        for (auto& p : m) {
            std::stringstream ss;
            p.first.print(ss, common::assignment_tuple);
            p.second[0].title = details::format_type(ss.str());
            res.push_back(std::move(p.second[0]));
        }
    }
    //! @brief Single page building.
    void build_impl(std::array<page, 1>& res, std::map<key_type, parent_build_type>& m) const {
        res[0].rows = m.size();
        res[0].cols = m.size() ? m.begin()->second.size() : 0;
        for (auto& p : m) {
            std::stringstream ss;
            p.first.print(ss, common::assignment_tuple);
            for (auto& q : p.second) {
                q.title = details::format_type(ss.str()) + (q.title.size() ? ", " : "") + q.title;
                res[0].plots.push_back(std::move(q));
            }
        }
    }
    //! @brief Multiple page building.
    void build_impl(std::vector<page>& res, std::map<key_type, parent_build_type>& m) const {
        for (auto& p : m) {
            std::stringstream ss;
            p.first.print(ss, common::assignment_tuple);
            for (auto& q : p.second) {
                q.title = details::format_type(ss.str()) + (q.title.size() ? ", " : "") + q.title;
                res.push_back(std::move(q));
            }
        }
    }

    //! @brief A mutex for synchronised access to the map.
    common::mutex<true> m_mutex;
    //! @brief The plotter.
    std::map<key_type, P> m_plotters;
};


/**
 * @brief Produces a single plot.
 *
 * @tparam S The sequence of tags and aggregators for logging (intertwined).
 * @tparam X The tag to be used for the x axis.
 * @tparam Y The unit to be used for the y axis.
 * @tparam A The sequence of row aggregators (defaults to `mean<double>`).
 */
template <typename S, typename X, template<class> class Y, typename A = common::type_sequence<>>
using plotter = split<X, values<S, A, unit<Y>>>;


}


}

#endif // FCPP_COMMON_PLOT_H_
