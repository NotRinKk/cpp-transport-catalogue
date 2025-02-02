#pragma once
#include <optional>
#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"


namespace request_handler {
class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);
    std::optional<domain::BusInfo> GetBusStat(const std::string_view& bus_name) const;
    std::optional<std::set<std::string_view, std::less<>>> GetBusesByStop(const std::string_view& stop_name) const;
    void RenderMap(std::ostringstream& svg_output) const;
    
private:
    std::set<domain::Stop*, domain::StopComparator> GetStops() const;
    std::set<const domain::Bus*, domain::BusComparator> GetBuses() const;
    std::vector<geo::Coordinates> GetStopsCoordinates() const;
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
}