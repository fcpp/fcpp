// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <vector>

#include "gtest/gtest.h"

#include "lib/common/flat_ptr.hpp"

using namespace fcpp;


TEST(FlatPtrTest, Size) {
    EXPECT_EQ(sizeof(char),                  sizeof(common::flat_ptr<char, true>));
    EXPECT_EQ(sizeof(std::shared_ptr<char>), sizeof(common::flat_ptr<char, false>));
}

TEST(FlatPtrTest, TrueOperators) {
    common::flat_ptr<char, true> data('a');
    common::flat_ptr<char, true> x(data), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}

TEST(FlatPtrTest, FalseOperators) {
    common::flat_ptr<char, false> data('a');
    common::flat_ptr<char, false> x(data), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}

TEST(FlatPtrTest, Dereferencing) {
    common::flat_ptr<std::vector<int>, false> fdata;
    common::flat_ptr<std::vector<int>, true> tdata;
    EXPECT_EQ(0, (int)fdata->size());
    EXPECT_EQ(0, (int)tdata->size());
    EXPECT_EQ(0, (int)(*fdata).size());
    EXPECT_EQ(0, (int)(*tdata).size());
}

TEST(FlatPtrTest, Assignment) {
    common::flat_ptr<char, false> fdata('a');
    common::flat_ptr<char, true> tdata('a');
    EXPECT_EQ('a', *fdata);
    EXPECT_EQ('a', *tdata);
    common::flat_ptr<char, false> f1 = fdata;
    common::flat_ptr<char, true> t1 = tdata;
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
