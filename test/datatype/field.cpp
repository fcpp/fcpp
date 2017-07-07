// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

#include <iostream>

#include "lib/datatype/field.hpp"


int main() {
    field<int> x(2, {{1,1},{6,-1}}), y(1,{{1,4},{2,3},{5,1}});
    std::cout << "x: " << x << std::endl;
    std::cout << "y: " << y << std::endl;
    std::cout << "x+y: " << x + y << std::endl;
    std::cout << "3+y: " << 3 + y << std::endl;
    std::cout << "x+1: " << x + 1 << std::endl;
    std::cout << "x*y: " << x * y << std::endl;
}
