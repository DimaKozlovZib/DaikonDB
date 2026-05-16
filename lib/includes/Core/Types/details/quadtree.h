#pragma once

#include <array>
#include <cmath>
#include <memory>
#include <vector>
#include <algorithm>

#include "../memory/TrackingAllocator.h"
#include "haversine.h"

namespace daikon::core::types::detail {

template <typename StringType, typename GeoPointType, typename Allocator>
class Quadtree {
   public:
    struct IndexedPoint {
        StringType name;
        GeoPointType point;
    };

    using PointAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<IndexedPoint>;

    Quadtree(const Allocator& alloc,
             size_t max_points_per_node = 8,
             size_t max_depth = 20,
             double min_node_size_m = 10.0)
        : node_alloc_(alloc),
          max_points_in_node_(max_points_per_node),
          max_depth_(max_depth),
          min_node_size_m_(min_node_size_m),
          nodes_(NodeAlloc(alloc)) {
        
        PointAlloc point_alloc(alloc);
        
        double root_size_m = 40075000.0; 

        nodes_.emplace_back(-90.0, 90.0, -180.0, 180.0, 0, root_size_m, point_alloc);
    }

    void Insert(const IndexedPoint& pt) {
        if (pt.point.longitude < -180.0 || pt.point.longitude > 180.0 ||
            pt.point.latitude < -90.0 || pt.point.latitude > 90.0) {
            return;
        }
        InsertInternal(0, pt);
    }

    std::vector<IndexedPoint> RadiusSearch(double lon, double lat,
                                           double radius_m) const {
        std::vector<IndexedPoint> result;
        result.reserve(100);

        double min_lat_box = lat - radius_m / 111319.9;
        double max_lat_box = lat + radius_m / 111319.9;

        double cos_lat = std::cos(lat * kPi / 180.0);
        double delta_lon = (cos_lat < 1e-6) ? 360.0 : radius_m / (111319.9 * cos_lat);

        std::array<LonInterval, 2> lon_intervals;
        size_t interval_count = 0;

        if (delta_lon >= 360.0) {
            lon_intervals[0] = {-180.0, 180.0};
            interval_count = 1;
        } else {
            double norm_lon = lon;
            while (norm_lon < -180.0) norm_lon += 360.0;
            while (norm_lon > 180.0) norm_lon -= 360.0;

            double r_min_lon = norm_lon - delta_lon;
            double r_max_lon = norm_lon + delta_lon;

            if (r_min_lon < -180.0) {
                lon_intervals[0] = {r_min_lon + 360.0, 180.0};
                lon_intervals[1] = {-180.0, r_max_lon};
                interval_count = 2;
            } else if (r_max_lon > 180.0) {
                lon_intervals[0] = {r_min_lon, 180.0};
                lon_intervals[1] = {-180.0, r_max_lon - 360.0};
                interval_count = 2;
            } else {
                lon_intervals[0] = {r_min_lon, r_max_lon};
                interval_count = 1;
            }
        }

        RadiusSearchInternal(0, min_lat_box, max_lat_box, lon_intervals, interval_count, lon, lat, radius_m, result);
        return result;
    }

   private:
    struct LonInterval { 
        double min_lon, max_lon; 
    };

    struct Node {
        double min_lat, max_lat, min_lon, max_lon;
        std::vector<IndexedPoint, PointAlloc> points;
        uint32_t first_child_idx = 0;
        size_t depth;
        double node_size_m;

        Node(double min_lat_, double max_lat_, double min_lon_, double max_lon_,
             size_t depth_, double node_size_m_, const PointAlloc& alloc)
            : min_lat(min_lat_), max_lat(max_lat_), min_lon(min_lon_), max_lon(max_lon_),
              points(alloc), first_child_idx(0), depth(depth_), node_size_m(node_size_m_) {}
    };

    using NodeAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    bool ContainsNode(const Node& node, double lon, double lat) const {
        bool lat_ok = (lat >= node.min_lat) && (node.max_lat == 90.0 ? lat <= node.max_lat : lat < node.max_lat);
        bool lon_ok = (lon >= node.min_lon) && (node.max_lon == 180.0 ? lon <= node.max_lon : lon < node.max_lon);
        return lat_ok && lon_ok;
    }

    void Subdivide(size_t node_idx) {
        double min_lat = nodes_[node_idx].min_lat;
        double max_lat = nodes_[node_idx].max_lat;
        double min_lon = nodes_[node_idx].min_lon;
        double max_lon = nodes_[node_idx].max_lon;
        size_t next_depth = nodes_[node_idx].depth + 1;
        double child_size_m = nodes_[node_idx].node_size_m * 0.5;

        double mid_lat = (min_lat + max_lat) * 0.5;
        double mid_lon = (min_lon + max_lon) * 0.5;

        uint32_t first_child = static_cast<uint32_t>(nodes_.size());
        nodes_[node_idx].first_child_idx = first_child;

        PointAlloc point_alloc = nodes_[node_idx].points.get_allocator();

        nodes_.emplace_back(min_lat, mid_lat, min_lon, mid_lon, next_depth, child_size_m, point_alloc);
        nodes_.emplace_back(min_lat, mid_lat, mid_lon, max_lon, next_depth, child_size_m, point_alloc);
        nodes_.emplace_back(mid_lat, max_lat, min_lon, mid_lon, next_depth, child_size_m, point_alloc);
        nodes_.emplace_back(mid_lat, max_lat, mid_lon, max_lon, next_depth, child_size_m, point_alloc);

        auto points_to_move = std::move(nodes_[node_idx].points);
        nodes_[node_idx].points.clear();
        nodes_[node_idx].points.shrink_to_fit();

        for (auto& pt : points_to_move) {
            bool distributed = false;
            for (uint32_t i = 0; i < 4; ++i) {
                size_t child_idx = first_child + i;
                if (ContainsNode(nodes_[child_idx], pt.point.longitude, pt.point.latitude)) {
                    nodes_[child_idx].points.push_back(std::move(pt));
                    distributed = true;
                    break;
                }
            }
            if (!distributed) {
                nodes_[first_child].points.push_back(std::move(pt));
            }
        }
    }

    void InsertInternal(size_t node_idx, const IndexedPoint& pt) {
        if (nodes_[node_idx].first_child_idx != 0) {
            uint32_t first_child = nodes_[node_idx].first_child_idx;
            for (uint32_t i = 0; i < 4; ++i) {
                size_t child_idx = first_child + i;
                if (ContainsNode(nodes_[child_idx], pt.point.longitude, pt.point.latitude)) {
                    InsertInternal(child_idx, pt);
                    return;
                }
            }
            InsertInternal(first_child, pt);
            return;
        }

        if (nodes_[node_idx].node_size_m <= min_node_size_m_ || nodes_[node_idx].depth >= max_depth_) {
            nodes_[node_idx].points.push_back(pt);
            return;
        }

        if (nodes_[node_idx].points.size() >= max_points_in_node_) {
            Subdivide(node_idx);
            InsertInternal(node_idx, pt);
        } else {
            nodes_[node_idx].points.push_back(pt);
        }
    }

    void RadiusSearchInternal(size_t node_idx, 
                              double min_lat_box, double max_lat_box,
                              const std::array<LonInterval, 2>& lon_intervals, size_t interval_count,
                              double q_lon, double q_lat, double radius_m,
                              std::vector<IndexedPoint>& out) const {
        const auto& node = nodes_[node_idx];

        if (node.max_lat < min_lat_box || node.min_lat > max_lat_box) {
            return;
        }

        bool lon_intersects = false;
        for (size_t i = 0; i < interval_count; ++i) {
            if (node.max_lon >= lon_intervals[i].min_lon && node.min_lon <= lon_intervals[i].max_lon) {
                lon_intersects = true;
                break;
            }
        }
        if (!lon_intersects) {
            return;
        }

        for (const auto& pt : node.points) {
            double d = HaversineMeters(q_lon, q_lat, pt.point.longitude, pt.point.latitude);
            if (d <= radius_m) {
                out.push_back(pt);
            }
        }

        if (node.first_child_idx != 0) {
            uint32_t first_child = node.first_child_idx;
            for (uint32_t i = 0; i < 4; ++i) {
                RadiusSearchInternal(first_child + i, min_lat_box, max_lat_box, lon_intervals, interval_count, q_lon, q_lat, radius_m, out);
            }
        }
    }

    NodeAlloc node_alloc_;
    std::vector<Node, NodeAlloc> nodes_;
    size_t max_points_in_node_;
    size_t max_depth_;
    double min_node_size_m_;
};

} // namespace daikon::core::types::detail