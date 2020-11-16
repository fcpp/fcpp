// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/plot.hpp"
#include "test/helper.hpp"

using namespace fcpp;

template <typename T>
struct temp {};

struct tag {};
struct gat {};
struct oth {};


#define EXPECT_POINT(q, u, s, v)    \
        EXPECT_EQ(q.unit, u);       \
        EXPECT_EQ(q.source, s);     \
        EXPECT_DOUBLE_EQ(q.value, v);


TEST(PlotTest, Value) {
    {
        plot::value<tag> p;
        p << common::make_tagged_tuple<plot::time,tag>(1.0, 2.0);
        p << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 4.0, 3.0);
        std::array<plot::point, 1> pb = p.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "", "tag (mean)", 3.0);
    }
    {
        plot::value<temp<tag>, aggregator::distinct<int>> p;
        p << common::make_tagged_tuple<plot::time,temp<tag>>(1.0, 2);
        p << common::make_tagged_tuple<plot::time,temp<tag>,gat>(0.0, 4, 3.0);
        p << common::make_tagged_tuple<plot::time,temp<tag>,gat>(2.0, 2, 1.0);
        std::array<plot::point, 1> pb = p.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "temp", "tag (distinct)", 2.0);
    }
    {
        plot::value<aggregator::mean<temp<tag>>, aggregator::distinct<int>> p;
        p << common::make_tagged_tuple<plot::time,aggregator::mean<temp<tag>>>(1.0, 2);
        p << common::make_tagged_tuple<plot::time,aggregator::mean<temp<tag>>,gat>(0.0, 4, 3.0);
        p << common::make_tagged_tuple<plot::time,aggregator::mean<temp<tag>>,gat>(2.0, 2, 1.0);
        std::array<plot::point, 1> pb = p.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "temp", "tag (mean-distinct)", 2.0);
    }
}

TEST(PlotTest, FilterValue) {
    plot::filter<plot::time, filter::above<10>, plot::value<tag>> p;
    p << common::make_tagged_tuple<plot::time,tag>(0.0, 2.0);
    p << common::make_tagged_tuple<plot::time,tag,gat>(10.0, 4.0, 3.0);
    p << common::make_tagged_tuple<plot::time,tag>(20.0, 6.0);
    std::array<plot::point, 1> pb = p.build();
    plot::point& q = pb[0];
    EXPECT_POINT(q, "", "tag (mean)", 5.0);
}

TEST(PlotTest, JoinValue) {
    plot::join<plot::value<tag>, plot::value<gat>> p;
    p << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 4.0, 3.0);
    p << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 6.0, 1.0);
    std::array<plot::point, 2> pb = p.build();
    EXPECT_POINT(pb[0], "", "tag (mean)", 5.0);
    EXPECT_POINT(pb[1], "", "gat (mean)", 2.0);
}

TEST(PlotTest, Values) {
    EXPECT_SAME(plot::values<common::type_sequence<tag, gat>, common::type_sequence<>>, plot::values<common::type_sequence<tag, gat>, common::type_sequence<aggregator::mean<double>>>);
    EXPECT_SAME(plot::values<common::type_sequence<tag, gat>, common::type_sequence<aggregator::mean<double>, aggregator::count<int>>>::build_type, std::array<plot::point, 4>);
    using aggr_t = common::type_sequence<
        tag,        aggregator::count<int>,
        gat,        aggregator::distinct<double>,
        gat,        aggregator::stats<double>,
        temp<tag>,  aggregator::mean<double>,
        temp<gat>,  aggregator::count<int>
    >;
    plot::values<aggr_t, common::type_sequence<>, gat, plot::unit<temp>, aggregator::count<int>> p;
    p << common::make_tagged_tuple<plot::time,aggregator::distinct<gat, true>, aggregator::mean<gat, true>, aggregator::deviation<gat, true>, aggregator::mean<temp<tag>, true>, aggregator::count<temp<gat>>, aggregator::count<tag>, gat>(0,1,2,3,4,5,6,7);
    std::array<plot::point, 7> pb = p.build();
    EXPECT_POINT(pb[0], "",     "gat (distinct-mean)", 1.0);
    EXPECT_POINT(pb[1], "",     "gat (mean-mean)",     2.0);
    EXPECT_POINT(pb[2], "",     "gat (dev-mean)",      3.0);
    EXPECT_POINT(pb[3], "temp", "tag (mean-mean)",     4.0);
    EXPECT_POINT(pb[4], "temp", "gat (count-mean)",    5.0);
    EXPECT_POINT(pb[5], "",     "tag (count-mean)",    6.0);
    EXPECT_POINT(pb[6], "temp", "gat (count-mean)",    5.0);
}

using splitjoinvalue = plot::split<plot::time, plot::join<plot::value<temp<tag>>, plot::value<temp<gat>>>>;

TEST(PlotTest, SplitJoinValue) {
    splitjoinvalue p;
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(0, 10, 0);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(1, 5,  5);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(2, 0, 10);
    std::array<plot::plot, 1> pb = p.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n");
}

using filtersplitvalue = plot::filter<plot::time, filter::above<1>, plot::split<temp<tag>, plot::value<temp<gat>>>>;

TEST(PlotTest, FilterSplitValue) {
    filtersplitvalue p;
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(0, 10, 0);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(1, 5,  5);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(2, 0, 10);
    std::array<plot::plot, 1> pb = p.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n");
    ss.str("");
    plot::file f("experiment", pb);
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\n\nshipout(\"experiment\");\n");
}

using joinfiltersplitjoinvalue = plot::join<splitjoinvalue, filtersplitvalue>;

TEST(PlotTest, JoinFilterSplitJoinValue) {
    joinfiltersplitjoinvalue p;
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(0, 10, 0);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(1, 5,  5);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(2, 0, 10);
    std::array<plot::plot, 2> pb = p.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n");
    ss.str("");
    ss << pb[1];
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n");
    ss.str("");
    plot::file f("experiment", pb);
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 1;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\n\nshipout(\"experiment\");\n");
}

using splitjoinfiltersplitjoinvalue = plot::split<oth, joinfiltersplitjoinvalue, std::ratio<10>>;

TEST(PlotTest, SplitJoinFilterSplitJoinValue) {
    splitjoinfiltersplitjoinvalue p;
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(0, 10, 0, -2);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(1, 5,  5, +1);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(2, 0, 10, +4);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(2, 10, 0, +6);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(1, 5,  5, +9);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(0, 0, 10, +13);
    std::array<plot::page, 1> pb = p.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-oth0\", \"oth = 0\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-oth0\", \"oth = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-oth10\", \"oth = 10\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-oth10\", \"oth = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n");
    ss.str("");
    plot::file f("experiment", pb);
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-oth0\", \"oth = 0\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-oth0\", \"oth = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-oth10\", \"oth = 10\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-oth10\", \"oth = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n\nshipout(\"experiment\");\n");
}

using joinsplitjoinfiltersplitjoinvalue = plot::join<splitjoinfiltersplitjoinvalue, filtersplitvalue>;

TEST(PlotTest, JoinSplitJoinFilterSplitJoinValue) {
    joinsplitjoinfiltersplitjoinvalue p;
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(0, 10, 0, -2);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(1, 5,  5, +1);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(2, 0, 10, +4);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(2, 10, 0, +6);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(1, 5,  5, +9);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth>(0, 0, 10, +13);
    std::array<plot::page, 2> pb = p.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-oth0\", \"oth = 0\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-oth0\", \"oth = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-oth10\", \"oth = 10\", \"time\", \"temp\", new string[] {\"tag (mean)\", \"gat (mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-oth10\", \"oth = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n");
    ss.str("");
    ss << pb[1];
    EXPECT_EQ(ss.str(), "plot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (mean)\"}, new pair[][] {{(0, 10), (5, 5), (10, 0)}}));\n\n");
}
