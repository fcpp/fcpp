#include <unordered_map>

#include "immutable_map.hpp"
#include "profiler.cpp"

fcpp::common::immutable_map<int, double> im;
std::unordered_map<int, double> um;

template <int k, typename M>
void add_stuff(M& m) {
    for (int i=k/4; i<k; ++i) {
        m.emplace(i, i*0.5);
    }
}

template <int k, typename M>
void query_stuff(M& m) {
    for (size_t i=0; i<k; ++i) {
        m.at((631*i)%k) += 1;
    }
}

template <typename M>
double final_check(M& m) {
    double sum = 0;
    for (auto& x : m) sum += x.first;
    return sum;
}

#define TEST(k, M) {   \
  M(#k); \
  { \
    M(#k "/immutable"); \
    {               \
        M(#k "/immutable/insert"); \
        add_stuff<k>(im); \
    } \
    {               \
        M(#k "/immutable/freeze"); \
        im.freeze(); \
    } \
    { \
        M(#k "/immutable/query"); \
        query_stuff<k>(im); \
    } \
  } \
  { \
    M(#k "/unordered"); \
    { \
        M(#k "/unordered/insert"); \
        add_stuff<k>(um); \
    } \
    { \
        M(#k "/unordered/query"); \
        query_stuff<k>(um); \
    } \
  } \
}

int main() {
    using namespace fcpp;
    for (int i=0; i<1000; ++i) {
        im = {};
        um = {};
        TEST(0x000001,);
        TEST(0x000004,);
        TEST(0x000010, PROFILE_COUNT);
        TEST(0x000040, PROFILE_COUNT);
        TEST(0x000100, PROFILE_COUNT);
        TEST(0x000400, PROFILE_COUNT);
        TEST(0x001000, PROFILE_COUNT);
        TEST(0x004000, PROFILE_COUNT);
        std::cerr << (final_check(im) == final_check(um)) << std::flush;
    }
    std::cerr << std::endl;
    PROFILE_REPORT();
}
