// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <vector>

#include "gtest/gtest.h"

#include "lib/internal/flat_ptr.hpp"

using namespace fcpp;


TEST(FlatPtrTest, Size) {
    EXPECT_EQ(sizeof(char),                  sizeof(internal::flat_ptr<char, true>));
    EXPECT_EQ(sizeof(std::shared_ptr<char>), sizeof(internal::flat_ptr<char, false>));
}

TEST(FlatPtrTest, TrueOperators) {
    internal::flat_ptr<char, true> data('a');
    internal::flat_ptr<char, true> x(data), y, z, a, b;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
    EXPECT_EQ(a, b);
    swap(z, a);
    EXPECT_EQ(data, a);
    EXPECT_EQ(z, b);
}

TEST(FlatPtrTest, FalseOperators) {
    internal::flat_ptr<char, false> data('a');
    internal::flat_ptr<char, false> x(data), y, z, a, b;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
    EXPECT_EQ(a, b);
    swap(z, a);
    EXPECT_EQ(data, a);
    EXPECT_EQ(z, b);
}

TEST(FlatPtrTest, Dereferencing) {
    internal::flat_ptr<std::vector<int>, false> fdata;
    internal::flat_ptr<std::vector<int>, true> tdata;
    EXPECT_EQ(0, (int)fdata->size());
    EXPECT_EQ(0, (int)tdata->size());
    EXPECT_EQ(0, (int)(*fdata).size());
    EXPECT_EQ(0, (int)(*tdata).size());
}

TEST(FlatPtrTest, Assignment) {
    internal::flat_ptr<char, false> fdata('a');
    internal::flat_ptr<char, true> tdata('a');
    EXPECT_EQ('a', *fdata);
    EXPECT_EQ('a', *tdata);
    internal::flat_ptr<char, false> f1 = fdata;
    internal::flat_ptr<char, true> t1 = tdata;
    EXPECT_EQ(*f1, *fdata);
    EXPECT_EQ(*t1, *tdata);
    EXPECT_EQ(f1, fdata);
    EXPECT_EQ(t1, tdata);
    *f1 = *t1 = 'z';
    EXPECT_EQ('z', *fdata);
    EXPECT_EQ('a', *tdata);
    f1 = 'g';
    t1 = 'g';
    EXPECT_EQ('g', *f1);
    EXPECT_EQ('g', *t1);
    EXPECT_EQ('z', *fdata);
    EXPECT_EQ('a', *tdata);
}
