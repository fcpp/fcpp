// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/common/profiler.hpp"

#ifdef PROFILER
std::clock_t fcpp::profiler::program_start{std::clock()};
std::unordered_map<std::string, std::clock_t> fcpp::profiler::clock_counters{};
#endif
