#pragma once
#include <optional>
#include "domain.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"


namespace request_handler {
    class RequestHandler {
    public:
        RequestHandler(const transport_catalogue::TransportCatalogue& db,
                    const renderer::MapRenderer& renderer,
                    const router::TransportRouter& routing);
        std::optional<domain::BusInfo> GetBusStat(const std::string_view& bus_name) const;
        std::optional<std::set<std::string_view, std::less<>>> GetBusesByStop(const std::string_view& stop_name) const;
        void RenderMap(std::ostringstream& svg_output) const;
        std::optional<router::RoutingResult> BuildRoute(std::string_view from, std::string_view to) const;
        
    private:
        std::set<domain::Stop*, domain::StopComparator> GetStops() const;
        std::set<const domain::Bus*, domain::BusComparator> GetBuses() const;
        std::vector<geo::Coordinates> GetStopsCoordinates() const;
        const transport_catalogue::TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
        const router::TransportRouter& routing_;
    };
} // namespace request_handler