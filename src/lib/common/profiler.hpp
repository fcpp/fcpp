// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file profiler.hpp
 * @brief Helper functions for profiling (if the `-DPROFILER` option is set).
 */

#ifndef FCPP_COMMON_PROFILER_H_
#define FCPP_COMMON_PROFILER_H_

#ifdef PROFILER

#include <ctime>

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Object accumulating the CPU elapsed during its lifetime in a given counter.
class profiler {
  public:
    //! @brief Constructor given a counter name.
    profiler(std::string n) : name{n}, start{std::clock()} {}

    //! @brief Destructor accumulating the CPU elapsed.
    ~profiler() {
        clock_counters[name] += std::clock() - start;
    }

    //! @brief Produces a final report for all counters.
    static void report(std::ostream& o = std::cerr) {
        std::clock_t tot = std::clock() - program_start;
        std::set<std::string> keys;
        size_t maxlen = 0;
        for (auto const& x : clock_counters) {
            keys.insert(x.first);
            maxlen = std::max(maxlen, x.first.size());
        }
        o << "ACTION";
        for (size_t i=3; i<maxlen; ++i) o << " ";
        o << "TOTAL     PARTIAL   SECS\n";
        for (auto const& k : keys) {
            o << k << ": ";
            for (size_t i=k.size(); i<maxlen; ++i) o << " ";
            int p = clock_counters[k]*10000/tot;
            if (p < 10000) o << " ";
            if (p < 1000) o << " ";
            o << p/100 << "." << (p/10)%10 << p%10 << "%";
            std::string pk = parent(k);
            if (pk != "") {
                p = clock_counters[k]*10000/clock_counters[pk];
                if (p < 10000) o << " ";
                if (p < 1000) o << " ";
                o << "   " << p/100 << "." << (p/10)%10 << p%10 << "%";
            } else o << "          ";
            double t = clock_counters[k] * 1.0 / CLOCKS_PER_SEC;
            o << "    ";
            if (t < 1000) o << " ";
            if (t < 100) o << " ";
            if (t < 10) o << " ";
            o << t << "\n";
        }
        o << "TOT";
        for (size_t i=0; i<maxlen+20; ++i) o << " ";
        double t = tot * 1.0 / CLOCKS_PER_SEC;
        if (t < 1000) o << " ";
        if (t < 100) o << " ";
        if (t < 10) o << " ";
        o << t << "\n";
        o.flush();
    }

  private:
    //! @brief Returns the parent counter of a given counter.
    static std::string parent(std::string s) {
        size_t x = s.rfind('/');
        return x == std::string::npos ? "" : s.substr(0, x);
    }

    //! @brief Stores the counter name.
    std::string name;
    //! @brief Stores the clock during construction.
    std::clock_t start;

    //! @brief Map storing counts for all counters.
    static std::unordered_map<std::string, std::clock_t> clock_counters;
    //! @brief The clock of program start.
    static std::clock_t program_start;
};

}

//! @brief Macro to be used for profiling (active version).
#define PROFILE_COUNT(...)  profiler profiling_counter{__VA_ARGS__}
//! @brief Macro to be used for report building (active version).
#define PROFILE_REPORT(...) profiler::report(__VA_ARGS__)

#else

//! @brief Macro to be used for profiling (disabled version).
#define PROFILE_COUNT(...)
//! @brief Macro to be used for report building (disabled version).
#define PROFILE_REPORT(...)

#endif

#endif // FCPP_COMMON_PROFILER_H_
