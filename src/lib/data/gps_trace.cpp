// Copyright © 2025 Lorenzo Framarin and Giorgio Audrito. All Rights Reserved.

#include "gps_trace.hpp"

#define _USE_MATH_DEFINES
#include <ctime>
#include <cmath>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <rapidxml.hpp>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


static constexpr real_t EARTH_RADIUS = 6371000.0;


gps_trace::gps_trace(const char* src_gpx_file, device_t ref_uid, real_t ref_lat, real_t ref_lon, real_t ref_ele, std::string const& ref_time) {
    std::ifstream file(src_gpx_file);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Failed to open GPX file: ") + src_gpx_file);
    }

    std::string xml((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    file.close();

    load(xml, ref_uid, ref_lat, ref_lon, ref_ele, ref_time);
}

gps_trace::gps_trace(std::string &xml, device_t ref_uid, real_t ref_lat, real_t ref_lon, real_t ref_ele, std::string const& ref_time) {
    load(xml, ref_uid, ref_lat, ref_lon, ref_ele, ref_time);
}

void gps_trace::load(std::string &xml, device_t ref_uid, real_t ref_lat, real_t ref_lon, real_t ref_ele, std::string const& ref_time) {
    times_t delta_time = parse_time(ref_time);
    device_t track_id = ref_uid;

    rapidxml::xml_document<> doc;
    doc.parse<0>(&xml[0]);
    rapidxml::xml_node<> *gpx_node = doc.first_node("gpx");
    if (!gpx_node) throw std::runtime_error("Failed to find gpx node");

    for (rapidxml::xml_node<> *trk_node = gpx_node->first_node("trk"); trk_node; trk_node = trk_node->next_sibling("trk")) {
        
        std::vector<track_point> track = {};

        for (rapidxml::xml_node<> *trkseg_node = trk_node->first_node("trkseg"); trkseg_node; trkseg_node = trkseg_node->next_sibling("trkseg")) {

            for (rapidxml::xml_node<> *trkpt_node = trkseg_node->first_node("trkpt"); trkpt_node; trkpt_node = trkpt_node->next_sibling("trkpt")) {

                rapidxml::xml_attribute<> *lat = trkpt_node->first_attribute("lat");
                rapidxml::xml_attribute<> *lon = trkpt_node->first_attribute("lon");
                rapidxml::xml_node<> *ele = trkpt_node->first_node("ele");
                rapidxml::xml_node<> *time = trkpt_node->first_node("time");

                if (lat && lon && time) {
                    real_t gps_lat = std::stod(lat->value());
                    real_t gps_lon = std::stod(lon->value());
                    track_point point = coord_to_meters(gps_lat, gps_lon, ref_lat, ref_lon);
                    point.z = ele ? std::stod(ele->value()) - ref_ele : 0.0;
                    point.timestamp = parse_time(time->value()) - delta_time;
                    track.push_back(point);
                }
            }
        }

        m_tracks[track_id] = track;
        ++track_id;
    }
};

track_point gps_trace::coord_to_meters(real_t lat, real_t lon, real_t ref_lat, real_t ref_lon) {
    track_point point;
    real_t ref_lat_rad = ref_lat * M_PI / 180;
    real_t d_lat = (lat - ref_lat) * M_PI / 180;
    real_t d_lon = (lon - ref_lon) * M_PI / 180;
    point.x = EARTH_RADIUS * std::cos(ref_lat_rad) * d_lon;
    point.y = EARTH_RADIUS * d_lat;
    return point;
}

times_t gps_trace::parse_time(std::string const& string) {
    int16_t year, mon, day, hour, min, sec;
    sscanf(string.c_str(), "%hd-%hd-%hdT%hd:%hd:%hd", &year, &mon, &day, &hour, &min, &sec);
    times_t res = day-1;
    res = res*24 + hour;
    res = res*60 + min;
    res = res*60 + sec;
    return res;
}

std::vector<track_point> const& gps_trace::find_track(device_t uid) {
    return m_tracks[uid];
}


} // namespace fcpp
