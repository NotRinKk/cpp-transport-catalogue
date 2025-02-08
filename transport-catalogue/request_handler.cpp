#include "request_handler.h"

namespace request_handler {
    using namespace domain;
    RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer) 
        : db_(db)
        , renderer_(renderer) {
    }

    std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        BusInfo result = db_.GetBusInfo(bus_name);
        if (result.exists) {
            return result;
        }
        else {
            return std::nullopt;
        }
    }

    std::optional<std::set<std::string_view, std::less<>>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        StopInfo result = db_.GetStopInfo(stop_name);
        if (result.exists) {
            std::set<std::string_view, std::less<>> buses;
            for (const auto& bus : result.buses) {
                buses.insert(bus);
            }
            return buses;
        }
        else {
            return std::nullopt;
        }
    }

    void RequestHandler::RenderMap(std::ostringstream& svg_output) const {

        const auto coords = GetStopsCoordinates();
        const renderer::SphereProjector proj(coords.begin(), coords.end(), renderer_.GetSettings().width
                                                                         , renderer_.GetSettings().height
                                                                         , renderer_.GetSettings().padding);                                                                                                                                     
        renderer_.Render(svg_output, std::move(proj),GetBuses(), GetStops());                      
    }

    std::set<Stop*, StopComparator> RequestHandler::GetStops() const {
        return db_.GetAllStopsWithBus();
    }
    std::set<const Bus*, BusComparator> RequestHandler::GetBuses() const {
        return db_.GetAllBuses();
    }
    
    std::vector<geo::Coordinates> RequestHandler::GetStopsCoordinates() const {
        return db_.GetAllStopsCoordinates();
    }
}