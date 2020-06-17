#include "project/tests/library.hpp"

#include <vector>
std::vector<int> skip_tags;

void push_back(int n) {
    skip_tags.push_back(n);
}

int pop_back() {
    int r = skip_tags.back();
    skip_tags.pop_back();
    return r;
}
