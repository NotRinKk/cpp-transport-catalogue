#pragma once

#include <iostream>
#include <memory>
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace router {
    struct Settings {
        double bus_velocity;
        int bus_wait_time;
    };

    struct RouteInfo {
        bool is_bus;
        std::string name;
        int span_count;
        double time;
    };

    struct RoutingResult {
        double travel_time;
        std::vector<RouteInfo> records;
    };

    
    struct RouteInternalInfo {
        size_t start_id;
        size_t end_id;
        std::string bus_name;
    };

    class TransportRouter {
    public:
        TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, router::Settings settings);
        std::optional<RoutingResult> BuildRoute(std::string_view from, std::string_view to) const;

    private:
        void BuildGraph();
        void AddBusEdges(const domain::Bus& bus);
        const transport_catalogue::TransportCatalogue& catalogue_;
        router::Settings settings_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        std::unordered_map<std::string_view, graph::VertexId> stop_to_vertex_;
        std::unordered_map<graph::VertexId, std::string_view> vertex_to_stop_;
        std::unordered_map<graph::EdgeId, RouteInternalInfo> edge_to_info_;

    };
} // namespace router