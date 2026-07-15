// Copyright © 2025 Lorenzo Framarin and Giorgio Audrito. All Rights Reserved.

/**
 * @file gps_trace.hpp
 * @brief Implementation and helper functions for the `gps_trace` class handling a collection of GPS traces.
 */

#ifndef FCPP_DATA_GPS_TRACE_HPP
#define FCPP_DATA_GPS_TRACE_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief A GPS track point.
struct track_point {
    //! @brief x coordinate.
    real_t x;
    //! @brief y coordinate.
    real_t y;
    //! @brief z coordinate.
    real_t z;
    //! @brief time coordinate.
    times_t timestamp;

    //! @brief Conversion to 2D vector.
    inline explicit operator vec<2>() const {
        return make_vec(x,y);
    }

    //! @brief Conversion to 3D vector.
    inline explicit operator vec<3>() const {
        return make_vec(x,y,z);
    }
};

/**
 * @brief Class handling a collection of GPS traces.
 */
class gps_trace {
  public: // visible by net objects and the main program
    //! @brief Default constructor.
    gps_trace() = default;

    /**
     * @brief Constructor from a GPX file path.
     *
     * @param src_gpx_file path of the gpx file to load.
     * @param ref_uid starting device id to use for new tracks.
     * @param ref_lat reference latitude to be mapped to x = 0.
     * @param ref_lon reference longitude to be mapped to y = 0.
     * @param ref_ele reference elevation to be mapped to z = 0.
     * @param ref_time reference time to be mapped to t = 0.
     */
    gps_trace(const char* src_gpx_file, device_t ref_uid, real_t ref_lat, real_t ref_lon, real_t ref_ele, std::string const& ref_time);

    /**
     * @brief Constructor from an XML string.
     *
     * @param xml XML string.
     * @param ref_uid starting device id to use for new tracks.
     * @param ref_lat reference latitude to be mapped to x = 0.
     * @param ref_lon reference longitude to be mapped to y = 0.
     * @param ref_ele reference elevation to be mapped to z = 0.
     * @param ref_time reference time to be mapped to t = 0.
     */
    gps_trace(std::string &xml, device_t ref_uid, real_t ref_lat, real_t ref_lon, real_t ref_ele, std::string const& ref_time);

    /**
     * @brief Load tracks from an XML string.
     *
     * @param xml XML string.
     * @param ref_uid starting device id to use for new tracks.
     * @param ref_lat reference latitude to be mapped to x = 0.
     * @param ref_lon reference longitude to be mapped to y = 0.
     * @param ref_ele reference elevation to be mapped to z = 0.
     * @param ref_time reference time to be mapped to t = 0.
     */
    void load(std::string &xml, device_t ref_uid, real_t ref_lat, real_t ref_lon, real_t ref_ele, std::string const& ref_time);

    /**
     * @brief Next track point to follow based on the given timestamp.
     *
     * Assumes that index is either valid, 0 or -1.
     *
     * @param uid the device uid.
     * @param index the start of the sub-track to consider.
     * @param next the time of the next round.
     * @param cur the time of the current round.
     * @param def the current position.
     */
    template <size_t n>
    vec<n> next_point(device_t uid, size_t& index, times_t next, times_t cur, vec<n> def) const {
        auto track = m_tracks.find(uid);
        if (track == m_tracks.end()) {
            index = (size_t)-1;
            return def;
        }
        for (; index < track->second.size() and track->second[index].timestamp < next; ++index);
        if (index >= track->second.size()) {
            index = (size_t)-1;
            return vec<n>(track->second.back());
        }
        if (track->second[index].timestamp == next)
            return vec<n>(track->second[index]);
        vec<n> vpre = index == 0 ? def : vec<n>(track->second[index-1]);
        vec<n> vpost = vec<n>(track->second[index]);
        real_t tpre = index == 0 ? cur : track->second[index-1].timestamp;
        real_t tpost = track->second[index].timestamp;
        return (vpre*(tpost-next) + vpost*(next-tpre))/(tpost-tpre);
    }

    //! @brief GPS trace associated with the given device id.
    std::vector<track_point> const& find_track(device_t uid);

    //! @brief Number of saved GPS tracks in the object.
    size_t size() const {
        return m_tracks.size();
    }

  private:
    //! @brief The stored GPS tracks.
    std::unordered_map<device_t, std::vector<track_point>> m_tracks;

    //! @brief Conversion of geographic coordinates to projected coordinates using equirectangular projection.
    static track_point coord_to_meters(real_t lat, real_t lon, real_t ref_lat, real_t ref_lon);

    /**
     * @brief Converts a time string from gpx file into a `times_t` value.
     * @param string timestamp given in ISO 8601 format (YYYY-MM-DDTHH:MM:SSZ).
     */
    static times_t parse_time(std::string const& string);
};


//! @brief Prints a `gps_traces` object.
template <typename O>
inline O& operator<<(O& os, gps_trace const& trace) {
    os << "gps_trace(" << trace.size() << " tracks)";
    return os;
}


} // namespace fcpp


#endif // FCPP_DATA_GPS_TRACE_HPP
