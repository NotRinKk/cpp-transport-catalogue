#include "transport_router.h"
namespace router {
    TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue, router::Settings settings)
        :   catalogue_(catalogue),
            settings_(settings),
            graph_(catalogue.GetAllStopsWithBus().size() * 2) {

            BuildGraph();
            router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    std::optional<RoutingResult> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
        auto from_it = stop_to_vertex_.find(from);
        auto to_it = stop_to_vertex_.find(to); 

        if (from_it == stop_to_vertex_.end() || to_it == stop_to_vertex_.end() ) {
            return std::nullopt; 
        }

        RoutingResult route{};
        if (from == to) {
            return route;
        }
         
        auto route_info = router_->BuildRoute(from_it->second, to_it->second);

        if (!route_info) {
            return std::nullopt;
        };
        route.travel_time = route_info->weight;

        for (const auto& id : route_info->edges) {
            const graph::Edge<double> route_edge = graph_.GetEdge(id);
            // Ожидание - если остановки соседние и вес равен времени ожидания
            if (route_edge.from + 1 == route_edge.to && domain::IsSame(route_edge.weight, settings_.bus_wait_time)) {
                // Ожидание
                route.records.emplace_back(RouteInfo{
                    .is_bus = false,
                    .name = std::string(vertex_to_stop_.at(route_edge.from)),
                    .span_count = 0u,
                    .time = static_cast<double>(settings_.bus_wait_time)
                });
            }
            else { 
                // Автобус
                const RouteInternalInfo& edge_info = edge_to_info_.at(id);
                route.records.emplace_back(RouteInfo{
                    .is_bus = true,
                    .name = edge_info.bus_name,
                    .span_count = static_cast<int>(edge_info.end_id - edge_info.start_id),
                    .time = route_edge.weight
                });
            }
        }
        return route;
    }    

    // Создаёт граф на основе данных справочника
    void TransportRouter::BuildGraph() {
        size_t vertex_id = 0;

        for (const auto* stop : catalogue_.GetAllStopsWithBus()) {

            vertex_to_stop_[vertex_id] = stop->stop_name;
            stop_to_vertex_[stop->stop_name] = vertex_id;

            graph_.AddEdge({
                .from = vertex_id,
                .to = vertex_id + 1,
                .weight = static_cast<double>(settings_.bus_wait_time)
            });
            vertex_id += 2;
        }

        for (const auto* bus : catalogue_.GetAllBuses()) {
            AddBusEdges(*bus);
        }    
    }

    void TransportRouter::AddBusEdges(const domain::Bus& bus) {
        const double velocity_mps = settings_.bus_velocity * 1000.0 / 60.0;
        const auto& stops = bus.stops;

        size_t stop_count = stops.size();
    
        if (stop_count < 2) return;
        
        // start - начальная остановка
        for (size_t start = 0; start < stop_count - 1; ++start) { 
            // Накопленное время
            double accumulated_travel_time = 0.0;
            // end - конечная остановка
            for (size_t end = start + 1; end < stop_count; ++end) { 
                double segment_distance = catalogue_.GetStopDistance(stops[end - 1], stops[end]);
                double segment_travel_time = segment_distance / velocity_mps;
    
                accumulated_travel_time += segment_travel_time;

                graph::EdgeId edge_id = graph_.AddEdge({
                    .from = stop_to_vertex_.at(stops[start]->stop_name) + 1, 
                    .to = stop_to_vertex_.at(stops[end]->stop_name), 
                    .weight = accumulated_travel_time
                });

                edge_to_info_[edge_id] = RouteInternalInfo{
                    .start_id = start,
                    .end_id = end,
                    .bus_name = bus.bus_name
                };
    
            }
        }
    }
} // namespace router