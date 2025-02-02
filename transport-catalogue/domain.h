#pragma once
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include "geo.h"

namespace domain {
    
    struct Stop	{
        std::string stop_name;
        geo::Coordinates coordinates;
    };
    struct Bus {
        bool is_roundtrip;
        std::string bus_name;
        std::vector<Stop*> stops;
    };

    struct BusInfo {
        bool exists;
        std::string_view name;
        size_t stops;
        size_t unique_stops;
        int distance;
        double curvature;
    };

    struct StopInfo {
        bool exists;
        std::string_view name;
        std::set<std::string_view> buses;
    };

    struct BusComparator {
        bool operator()(const Bus* lhs, const Bus* rhs) const {
            return lhs->bus_name < rhs->bus_name;
        }
    };

    struct StopComparator {
        bool operator()(const Stop* lhs, const Stop* rhs) const {
            return lhs->stop_name < rhs->stop_name;
        }
    };
}