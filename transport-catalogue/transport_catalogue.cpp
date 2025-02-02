#include "transport_catalogue.h"

namespace transport_catalogue {
    std::set<Stop*, StopComparator> TransportCatalogue::GetAllStopsWithBus() const {
        std::set<Stop*, StopComparator> stops;
        for (const auto& stop : buses_to_stop_) {
            if (!stop.second.empty()) {
                stops.insert(FindStopByName(stop.first));
            }
        }
        return stops;
    }

    std::set<const Bus*, domain::BusComparator> TransportCatalogue::GetAllBuses() const {
        std::set<const Bus*, domain::BusComparator> sorted_buses;
        for (const auto& bus : buses_) {
            sorted_buses.insert(&bus);
        }
        return sorted_buses;
    }
    std::vector<geo::Coordinates> TransportCatalogue::GetAllStopsCoordinates() const {
        std::vector<geo::Coordinates> geo_coord;
        for (const auto& stop : stops_) {
            if(!GetStopInfo(stop.stop_name).buses.empty()) {
                geo_coord.push_back(stop.coordinates);    
            }
        }
        return geo_coord;
    }

    void TransportCatalogue::AddStopDistances(std::string_view stop, std::vector<std::pair<int, std::string>> distances_stops) {
        auto itl = index_stops_.find(stop);

        for (const auto& [distance, neighbor_stop] : distances_stops) {
            stops_distance_[std::make_pair((*itl).second, (*index_stops_.find(neighbor_stop)).second)] = distance;
        }
    }

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        index_stops_[stops_.back().stop_name] = &stops_.back();
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        auto added_bus = &buses_.back();
        index_buses_[buses_.back().bus_name] = added_bus;

        for (const auto stop : bus.stops) {
            buses_to_stop_[stop->stop_name].emplace(added_bus->bus_name);
        }
    }

    Bus* TransportCatalogue::FindBusByName(std::string_view name) const {
        auto it = index_buses_.find(name);
        if(it == index_buses_.end()) {
            return nullptr;
        }
        return (*it).second;
    }

    Stop* TransportCatalogue::FindStopByName(std::string_view name) const {
        auto it = index_stops_.find(name);
        if (it == index_stops_.end())         {
            return nullptr;
        }
        return (*it).second;
    }

    size_t TransportCatalogue::CountUniqueStopsByBus(Bus* bus) const { 
        std::unordered_set<Stop*> unique_stops;
        for (const auto stop : bus->stops) {
            unique_stops.insert(stop);
        }
        return unique_stops.size();
    }

    double  TransportCatalogue::GetBusDistance(Bus* bus) const  {
        double distance = 0.;
        for (auto it = bus->stops.cbegin(); it != bus->stops.cend() - 1; ++it) {
            auto it_next = next(it);
            distance += ComputeDistance((*it)->coordinates, (*it_next)->coordinates);
        }
        return distance;
    }

    BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        using namespace std;

        string info;
        auto bus = FindBusByName(bus_name);
        if (bus == nullptr) {
            BusInfo bus_info{ 
                             false,
                             bus_name,
                             0u,
                             0u,
                             0,
                             0. 
            };
            return bus_info;
        }

        size_t unique_stops = CountUniqueStopsByBus(bus);

        int distance = GetBusFactDistance (bus);
        double curvature = CalculateCurvature(bus);

        BusInfo bus_info{
                          true,
                          bus_name,
                          bus->stops.size(),
                          unique_stops,
                          distance,
                          curvature 
        };

        return bus_info;
    }

    int TransportCatalogue::GetBusFactDistance(Bus* bus) const {
        int distance = 0;
        for (size_t stop = 0; stop + 1 < bus->stops.size(); ++stop) {
            auto it = stops_distance_.find(std::make_pair(bus->stops[stop], bus->stops[stop + 1]));

            if (it == stops_distance_.end()) {
                it = stops_distance_.find(std::make_pair(bus->stops[stop + 1], bus->stops[stop]));
            }

            distance += it->second;
        }
        return distance;
    }

    double TransportCatalogue::CalculateCurvature(Bus* bus) const {
        return static_cast<double>(GetBusFactDistance(bus)) / GetBusDistance(bus);
    }

    StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
        using namespace std;

        auto stop = FindStopByName(stop_name);
        if (stop == nullptr) {
            StopInfo stop_info{
                                false,
                                stop_name,
                                std::set<std::string_view>()
            };
            return stop_info;
        }

        const auto it = buses_to_stop_.find(stop_name);
        if (it == buses_to_stop_.end()) {
            StopInfo stop_info{
                                true,
                                stop_name,
                                std::set<std::string_view>()
            };
            return stop_info;
        }

        StopInfo stop_info{
                                true,
                                stop_name,
                                std::set<std::string_view>()
        };
        for (const auto bus_name : (*it).second) {
            stop_info.buses.insert(bus_name);
        }

        return stop_info;
    }

    void TransportCatalogue::AddAllDistancesToStops(std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> stop_to_distances) {
        for (const auto& stop : stop_to_distances) {
            AddStopDistances(stop.first, std::move(stop.second));
        }
    }
}