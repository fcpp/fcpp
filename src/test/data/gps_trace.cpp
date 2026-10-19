// Copyright © 2025 Lorenzo Framarin and Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/gps_trace.hpp"

#include <rapidxml.hpp>


using namespace fcpp;

TEST(GpsTraceTest, Parsing) {
    std::string gpx_data = std::string(R"(<?xml version="1.0"?>
<gpx>
  <trk>
    <trkseg>
      <trkpt lat="47.86675" lon="15.16357">
        <ele>833.15</ele>
        <time>2018-05-21T04:51:34Z</time>
      </trkpt>
      <trkpt lat="47.86712" lon="15.16338">
        <ele>833.27</ele>
        <time>2018-05-21T04:52:14Z</time>
      </trkpt>
      <trkpt lat="47.86772" lon="15.16324">
        <ele>834.12</ele>
        <time>2018-05-21T04:52:56Z</time>
      </trkpt>
    </trkseg>
  </trk>
  <trk>
    <trkseg>
      <trkpt lat="47.86800" lon="15.16400">
        <time>2018-05-21T05:51:34Z</time>
      </trkpt>
      <trkpt lat="47.86850" lon="15.16450">
        <time>2018-05-21T05:52:14Z</time>
      </trkpt>
    </trkseg>
  </trk>
</gpx>)");

    static gps_trace trace(gpx_data, 0, 47.86675, 15.16357, 833.15, "2018-05-21T04:51:00Z");

    EXPECT_EQ(trace.size(), 2);

    std::vector<track_point> const& td_a = trace.find_track(0);
    std::vector<track_point> const& td_b = trace.find_track(1);

    ASSERT_FALSE(td_a.empty());
    ASSERT_FALSE(td_b.empty());
    EXPECT_EQ(td_a.size(), 3);
    EXPECT_EQ(td_b.size(), 2);

    //check track points values to verify correct conversions
    EXPECT_EQ(td_a[0].x, 0);
    EXPECT_EQ(td_a[0].y, 0);
    EXPECT_EQ(td_a[0].z, 0);
    EXPECT_EQ(td_a[0].timestamp, 34);
    EXPECT_LT(td_a[1].x, 0.0);
    EXPECT_GT(td_a[1].y, 0.0);
    EXPECT_GT(td_a[1].z, 0.0);
    EXPECT_EQ(td_a[1].timestamp, 74);
}

TEST(GpsTraceTest, Navigation) {
    std::string gpx_data = std::string(R"(<?xml version="1.0"?>
<gpx>
  <trk>
    <trkseg>
      <trkpt lat="47.86675" lon="15.16357">
        <ele>833.15</ele>
        <time>2018-05-21T04:51:34Z</time>
      </trkpt>
      <trkpt lat="47.86712" lon="15.16338">
        <ele>833.27</ele>
        <time>2018-05-21T04:52:14Z</time>
      </trkpt>
      <trkpt lat="47.86772" lon="15.16324">
        <ele>834.12</ele>
        <time>2018-05-21T04:52:56Z</time>
      </trkpt>
    </trkseg>
  </trk>
</gpx>)");

    static gps_trace trace(gpx_data, 0, 47.86675, 15.16357, 833.15, "2018-05-21T04:51:00Z");

    EXPECT_EQ(trace.size(), 1);

    EXPECT_EQ(trace.find_track(0).size(), 3);

    size_t index = 0;
    trace.next_point(0, index, 30, 0, make_vec(0,0));
    EXPECT_EQ(index, 0);
    trace.next_point(0, index, 60, 30, make_vec(0,0));
    EXPECT_EQ(index, 1);
    trace.next_point(0, index, 74, 30, make_vec(0,0));
    EXPECT_EQ(index, 1);
    trace.next_point(0, index, 200, 30, make_vec(0,0));
    EXPECT_EQ(index, (size_t)-1);
}

TEST(GpsTraceTest, InvalidGpx) {
    std::string invalid_gpx = "Lorem Ipsum";
    
    EXPECT_THROW({
        gps_trace trace(invalid_gpx, 0, 0.0, 0.0, 0.0, "2018-05-21T04:51:00Z");
    }, rapidxml::parse_error);
}

TEST(GpsTraceTest, EmptyGpx) {
    std::string empty_gpx = R"(<?xml version="1.0"?>
<gpx>
</gpx>)";
    
    gps_trace trace(empty_gpx, 0, 0.0, 0.0, 0.0, "2018-05-21T04:51:00Z");
    EXPECT_EQ(trace.size(), 0);
}
