// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/plot.hpp"
#include "lib/option/filter.hpp"
#include "test/helper.hpp"

using namespace fcpp;

template <typename T>
struct temp {};
template <typename... Ts>
struct temps {};

struct tag {};
struct gat {};
struct oth_but {};
struct but_oth {};


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
        EXPECT_POINT(q, "tag", "tag (finite mean)", 3.0);
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
    {
        plot::value<aggregator::mean<temp<tag>>, aggregator::distinct<int>> p1, p2;
        p1 << common::make_tagged_tuple<plot::time,aggregator::mean<temp<tag>>>(1.0, 2);
        p2 << common::make_tagged_tuple<plot::time,aggregator::mean<temp<tag>>,gat>(0.0, 4, 3.0);
        p2 << common::make_tagged_tuple<plot::time,aggregator::mean<temp<tag>>,gat>(2.0, 2, 1.0);
        p1 += p2;
        std::array<plot::point, 1> pb = p1.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "temp", "tag (mean-distinct)", 2.0);
    }
}

TEST(PlotTest, FilterValue) {
    {
        plot::filter<plot::time, filter::above<10>, plot::value<tag>> p;
        p << common::make_tagged_tuple<plot::time,tag>(0.0, 2.0);
        p << common::make_tagged_tuple<plot::time,tag,gat>(10.0, 4.0, 3.0);
        p << common::make_tagged_tuple<plot::time,tag>(20.0, 6.0);
        std::array<plot::point, 1> pb = p.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "tag", "tag (finite mean)", 5.0);
    }
    {
        plot::filter<plot::time, filter::above<10>, plot::value<tag>> p1, p2;
        p1 << common::make_tagged_tuple<plot::time,tag>(0.0, 2.0);
        p2 << common::make_tagged_tuple<plot::time,tag,gat>(10.0, 4.0, 3.0);
        p2 << common::make_tagged_tuple<plot::time,tag>(20.0, 6.0);
        p1 += p2;
        std::array<plot::point, 1> pb = p1.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "tag", "tag (finite mean)", 5.0);
    }
    {
        plot::filter<plot::time, filter::above<10>, gat, filter::below<5>, plot::value<tag>> p;
        p << common::make_tagged_tuple<plot::time,tag,gat>(0.0,  1.0, 1.0);
        p << common::make_tagged_tuple<plot::time,tag,gat>(5.0,  2.0, 9.0);
        p << common::make_tagged_tuple<plot::time,tag,gat>(10.0, 4.0, 3.0);
        p << common::make_tagged_tuple<plot::time,tag,gat>(20.0, 8.0, 6.0);
        std::array<plot::point, 1> pb = p.build();
        plot::point& q = pb[0];
        EXPECT_POINT(q, "tag", "tag (finite mean)", 4.0);
    }
}

TEST(PlotTest, JoinValue) {
    {
        plot::join<plot::value<tag>, plot::value<gat>> p;
        p << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 4.0, 3.0);
        p << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 6.0, 1.0);
        std::array<plot::point, 2> pb = p.build();
        EXPECT_POINT(pb[0], "tag", "tag (finite mean)", 5.0);
        EXPECT_POINT(pb[1], "gat", "gat (finite mean)", 2.0);
    }
    {
        plot::join<plot::value<tag>, plot::value<gat>> p1, p2;
        p1 << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 4.0, 3.0);
        p2 << common::make_tagged_tuple<plot::time,tag,gat>(0.0, 6.0, 1.0);
        p1 += p2;
        std::array<plot::point, 2> pb = p1.build();
        EXPECT_POINT(pb[0], "tag", "tag (finite mean)", 5.0);
        EXPECT_POINT(pb[1], "gat", "gat (finite mean)", 2.0);
    }
}

TEST(PlotTest, Values) {
    EXPECT_SAME(plot::values<common::type_sequence<tag, gat>, common::type_sequence<>>, plot::values<common::type_sequence<tag, gat>, common::type_sequence<aggregator::only_finite<aggregator::mean<double>>>>);
    EXPECT_SAME(plot::values<common::type_sequence<tag, gat>, common::type_sequence<aggregator::mean<double>, aggregator::count<int>>>::build_type, std::array<plot::point, 4>);
    using aggr_t = common::type_sequence<
        tag,        aggregator::count<int>,
        gat,        aggregator::distinct<double>,
        gat,        aggregator::stats<double>,
        temp<tag>,  aggregator::mean<double>,
        temp<gat>,  aggregator::count<int>
    >;
    plot::values<aggr_t, common::type_sequence<>, gat, plot::unit<temp>, aggregator::count<int>> p;
    p << common::make_tagged_tuple<plot::time,aggregator::distinct<gat>, aggregator::mean<gat>, aggregator::deviation<gat>, aggregator::mean<temp<tag>>, aggregator::count<temp<gat>>, aggregator::count<tag>, gat>(0,1,2,3,4,5,6,7);
    std::array<plot::point, 7> pb = p.build();
    EXPECT_POINT(pb[0], "gat",  "gat (distinct-finite mean)", 1.0);
    EXPECT_POINT(pb[1], "gat",  "gat (mean-finite mean)",     2.0);
    EXPECT_POINT(pb[2], "gat",  "gat (dev-finite mean)",      3.0);
    EXPECT_POINT(pb[3], "temp", "tag (mean-finite mean)",     4.0);
    EXPECT_POINT(pb[4], "temp", "gat (count-finite mean)",    5.0);
    EXPECT_POINT(pb[5], "tag",  "tag (count-finite mean)",    6.0);
    EXPECT_POINT(pb[6], "temp", "gat (count-finite mean)",    5.0);
}

using splitjoinvalue = plot::split<plot::time, plot::join<plot::value<temp<tag>>, plot::value<temp<gat>>>>;

TEST(PlotTest, SplitJoinValue) {
    {
        splitjoinvalue p;
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(0, 10, 0);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(1, 5,  5);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(2, 0, 10);
        std::array<plot::plot, 1> pb = p.build();
        std::stringstream ss;
        ss << pb[0];
        EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n");
    }
    {
        splitjoinvalue p1, p2;
        p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(0, 10, 0);
        p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(1, 5,  5);
        p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>>(2, 0, 10);
        p1 += p2;
        std::array<plot::plot, 1> pb = p1.build();
        std::stringstream ss;
        ss << pb[0];
        EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n");
    }
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
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n");
    ss.str("");
    plot::file f("experiment", pb);
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\n\nshipout(\"experiment\");\n");
    ss.str("");
    f = plot::file("experiment", pb, {{"SUBPLOT", "true"}, {"LOG_LIN", "1"}});
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.SUBPLOT = true;\nplot.LOG_LIN = 1;\n\nplot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\n\nshipout(\"experiment\");\n");
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
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n");
    ss.str("");
    ss << pb[1];
    EXPECT_EQ(ss.str(), "plot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n");
    ss.str("");
    plot::file f("experiment", pb);
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 1;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp\", \"\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\n\nshipout(\"experiment\");\n");
}

using splitjoinfiltersplitjoinvalue = plot::split<oth_but, joinfiltersplitjoinvalue, std::ratio<10>>;

TEST(PlotTest, SplitJoinFilterSplitJoinValue) {
    splitjoinfiltersplitjoinvalue p;
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(0, 10, 0, -2);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(1, 5,  5, +1);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(2, 0, 10, +4);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(2, 10, 0, +6);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(1, 5,  5, +9);
    p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(0, 0, 10, +13);
    std::array<plot::page, 1> pb = p.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-obut0\", \"oth but = 0\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut0\", \"oth but = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut10\", \"oth but = 10\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut10\", \"oth but = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n");
    ss.str("");
    plot::file f("experiment", pb);
    ss << f;
    EXPECT_EQ(ss.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-obut0\", \"oth but = 0\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut0\", \"oth but = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut10\", \"oth but = 10\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut10\", \"oth but = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n\nshipout(\"experiment\");\n");
}

using multisplitjoinfiltersplitjoinvalue = plot::split<common::type_sequence<oth_but, but_oth>, joinfiltersplitjoinvalue, std::ratio<10>>;

TEST(PlotTest, MultiSplitJoinFilterSplitJoinValue) {
    multisplitjoinfiltersplitjoinvalue p1, p2;
    p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but, but_oth>(0, 10, 0, -2, 19);
    p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but, but_oth>(1, 5,  5, +1, 24);
    p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but, but_oth>(2, 0, 10, +4, 31);
    p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but, but_oth>(2, 10, 0, +6, 26);
    p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but, but_oth>(1, 5,  5, +9, 20);
    p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but, but_oth>(0, 0, 10, +13,23);
    p1 += p2;
    std::array<plot::page, 1> pb = p1.build();
    std::stringstream ss;
    ss << pb[0];
    EXPECT_EQ(ss.str(), "plot.ROWS = 4;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-obut0both20\", \"oth but = 0, but oth = 20\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5)}, {(0, 0), (1, 5)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut0both20\", \"oth but = 0, but oth = 20\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut0both30\", \"oth but = 0, but oth = 30\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(2, 0)}, {(2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut0both30\", \"oth but = 0, but oth = 30\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut10both20\", \"oth but = 10, but oth = 20\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 0), (1, 5)}, {(0, 10), (1, 5)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut10both20\", \"oth but = 10, but oth = 20\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut10both30\", \"oth but = 10, but oth = 30\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(2, 10)}, {(2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut10both30\", \"oth but = 10, but oth = 30\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(10, 0)}}));\n\n");
}

using joinsplitjoinfiltersplitjoinvalue = plot::join<splitjoinfiltersplitjoinvalue, filtersplitvalue>;

TEST(PlotTest, JoinSplitJoinFilterSplitJoinValue) {
    {
        joinsplitjoinfiltersplitjoinvalue p;
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(0, 10, 0, -2);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(1, 5,  5, +1);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(2, 0, 10, +4);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(2, 10, 0, +6);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(1, 5,  5, +9);
        p << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(0, 0, 10, +13);
        std::array<plot::page, 2> pb = p.build();
        std::stringstream ss;
        ss << pb[0];
        EXPECT_EQ(ss.str(), "plot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-obut0\", \"oth but = 0\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut0\", \"oth but = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut10\", \"oth but = 10\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut10\", \"oth but = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n");
        ss.str("");
        ss << pb[1];
        EXPECT_EQ(ss.str(), "plot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5), (10, 0)}}));\n\n");
    }
    {
        joinsplitjoinfiltersplitjoinvalue p1, p2;
        p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(0, 10, 0, -2);
        p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(1, 5,  5, +1);
        p1 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(2, 0, 10, +4);
        p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(2, 10, 0, +6);
        p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(1, 5,  5, +9);
        p2 << common::make_tagged_tuple<plot::time, temp<tag>, temp<gat>, oth_but>(0, 0, 10, +13);
        p1 += p2;
        std::array<plot::page, 2> pb = p1.build();
        std::stringstream ss;
        ss << pb[0];
        EXPECT_EQ(ss.str(), "plot.ROWS = 2;\nplot.COLS = 2;\n\nplot.put(plot.plot(name+\"-timtemp-obut0\", \"oth but = 0\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 10), (1, 5), (2, 0)}, {(0, 0), (1, 5), (2, 10)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut0\", \"oth but = 0\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5)}}));\n\nplot.put(plot.plot(name+\"-timtemp-obut10\", \"oth but = 10\", \"time\", \"temp\", new string[] {\"tag (finite mean)\", \"gat (finite mean)\"}, new pair[][] {{(0, 0), (1, 5), (2, 10)}, {(0, 10), (1, 5), (2, 0)}}));\n\nplot.put(plot.plot(name+\"-ttagtemp-obut10\", \"oth but = 10\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(5, 5), (10, 0)}}));\n\n");
        ss.str("");
        ss << pb[1];
        EXPECT_EQ(ss.str(), "plot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-ttagtemp\", \"\", \"temp<tag>\", \"temp\", new string[] {\"gat (finite mean)\"}, new pair[][] {{(0, 10), (5, 5), (10, 0)}}));\n\n");
    }
}

#define EXPECT_SERIALIZE(a, b, s) \
        EXPECT_EQ(tester(a, b), std::make_tuple(s, a, b))

TEST(PlotTest, DeltaTuple) {
    using tuple_type = plot::details::delta_tuple<common::tagged_tuple_t<tag, int, gat, double>>;

    tuple_type t;
    t = common::make_tagged_tuple<tag, gat>(2, 3.5);

    auto tester = [&](int a, double b){
        tuple_type u(t);
        u = common::make_tagged_tuple<tag, gat>(a, b);
        common::osstream os;
        os << u;
        tuple_type s(t);
        common::isstream is(os);
        is >> s;
        return std::make_tuple(os.size(), get<tag>(s), get<gat>(s));
    };

    EXPECT_SERIALIZE(2, 3.5, 1);
    EXPECT_SERIALIZE(3, 3.5, 1+sizeof(int));
    EXPECT_SERIALIZE(2, 1.5, 1+sizeof(double));
    EXPECT_SERIALIZE(3, 1.5, 1+sizeof(int)+sizeof(double));
}

TEST(PlotTest, Rows) {
    EXPECT_SAME(typename plot::details::option_types<void>::type, common::type_sequence<>);
    EXPECT_SAME(typename plot::details::option_types<common::type_sequence<int, bool>>::type, common::type_sequence<int, bool>);
    EXPECT_SAME(typename plot::details::option_types<temps<int, bool>>::type, common::type_sequence<int, bool>);
    {
        using plot_t = plot::rows<temps<tag, int, gat, double>>;
        EXPECT_SAME(plot_t::compressible_tuple_type, plot::details::delta_tuple<common::tagged_tuple_t<tag, int, gat, double>>);
        EXPECT_SAME(plot_t::mutable_tuple_type, common::tagged_tuple_t<>);
        EXPECT_SAME(plot_t::fixed_tuple_type, common::tagged_tuple_t<>);
        EXPECT_EQ(plot_t::limit_size, size_t{0});
        plot_t p;
        p << common::make_tagged_tuple<tag,gat,oth_but>(1, 2.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(1, 3.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(42, 3.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(42, 4.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(42, 5.5, true);
        EXPECT_EQ(p.size(), 5ULL);
        EXPECT_EQ(p.byte_size(), sizeof(plot_t) + 5 + sizeof(int)*2 + sizeof(double)*4);
        std::vector<std::string> v = {
            "########################################################",
            "# FCPP execution started at:  Fri Nov 20 13:22:29 2020 #",
            "########################################################",
            "# ",
            "#",
            "# The columns have the following meaning:",
            "# tag gat ",
            "1 2.5 ",
            "1 3.5 ",
            "42 3.5 ",
            "42 4.5 ",
            "42 5.5 ",
            "########################################################",
            "# FCPP execution finished at: Fri Nov 20 13:22:29 2020 #",
            "########################################################"
        };
        std::stringstream ss;
        p.print(ss);
        for (size_t i=0; i<v.size(); ++i) {
            std::string line;
            getline(ss, line);
            if (i != 1 and i != v.size()-2) {
                EXPECT_EQ(line, v[i]);
            }
        }
        ss.str("");
        p.print(ss);
        for (size_t i=0; i<v.size(); ++i) {
            std::string line;
            getline(ss, line);
            if (i != 1 and i != v.size()-2) {
                EXPECT_EQ(line, v[i]);
            }
        }
    }
    {
        using plot_t = plot::rows<temps<tag, int>, temps<gat, double>, temps<oth_but, bool>, 50>;
        EXPECT_SAME(plot_t::compressible_tuple_type, plot::details::delta_tuple<common::tagged_tuple_t<tag, int>>);
        EXPECT_SAME(plot_t::mutable_tuple_type, common::tagged_tuple_t<gat, double>);
        EXPECT_SAME(plot_t::fixed_tuple_type, common::tagged_tuple_t<oth_but, bool>);
        EXPECT_EQ(plot_t::limit_size, size_t{50});
        plot_t p;
        p << common::make_tagged_tuple<tag,gat,oth_but>(1, 2.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(1, 3.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(42, 3.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(42, 4.5, true);
        p << common::make_tagged_tuple<tag,gat,oth_but>(42, 5.5, true);
        EXPECT_EQ(p.size(), 4ULL);
        EXPECT_EQ(p.byte_size(), sizeof(plot_t) + 4 + sizeof(int)*2 + sizeof(double)*4);
        std::vector<std::string> v = {
            "########################################################",
            "# FCPP execution started at:  Fri Nov 20 13:34:18 2020 #",
            "########################################################",
            "# oth_but = true",
            "#",
            "# The columns have the following meaning:",
            "# gat tag ",
            "2.5 1 ",
            "3.5 1 ",
            "3.5 42 ",
            "4.5 42 ",
            "########################################################",
            "# FCPP execution finished at: Fri Nov 20 13:34:18 2020 #",
            "########################################################"
        };
        std::stringstream ss;
        p.print(ss);
        for (size_t i=0; i<v.size(); ++i) {
            std::string line;
            getline(ss, line);
            if (i != 1 and i != v.size()-2) {
                EXPECT_EQ(line, v[i]);
            }
        }
        ss.str("");
        p.print(ss);
        for (size_t i=0; i<v.size(); ++i) {
            std::string line;
            getline(ss, line);
            if (i != 1 and i != v.size()-2) {
                EXPECT_EQ(line, v[i]);
            }
        }
    }
}
