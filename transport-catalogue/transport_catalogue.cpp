#include "transport_catalogue.h"

namespace transport_catalogue {
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

    TransportCatalogue::Bus* TransportCatalogue::FindBusByName(std::string_view name) const {
        auto it = index_buses_.find(name);
        if(it == index_buses_.end()) {
            return nullptr;
        }
        return (*it).second;
    }

    TransportCatalogue::Stop* TransportCatalogue::FindStopByName(std::string_view name) const {
        auto it = index_stops_.find(name);
        if (it == index_stops_.end())         {
            return nullptr;
        }
        return (*it).second;
    }

    size_t TransportCatalogue::CountUniqueStopsByBus(TransportCatalogue::Bus* bus) const { 
        std::unordered_set<Stop*> unique_stops;
        for (const auto stop : bus->stops) {
            unique_stops.insert(stop);
        }
        return unique_stops.size();
    }

    double  TransportCatalogue::GetBusDistance(TransportCatalogue::Bus* bus) const  {
        double distance = 0.;
        for (auto it = bus->stops.cbegin(); it != bus->stops.cend() - 1; ++it) {
            auto it_next = next(it);
            distance += ComputeDistance((*it)->coordinates, (*it_next)->coordinates);
        }
        return distance;
    }

    TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        using namespace std;

        string info;
        auto bus = FindBusByName(bus_name);
        if (bus == nullptr) {
            BusInfo bus_info{ 
                             false,
                             bus_name,
                             0u,
                             0u,
                             0. 
            };
            return bus_info;
        }

        size_t unique_stops = CountUniqueStopsByBus(bus);
        double distance = GetBusDistance(bus);

        BusInfo bus_info{
                          true,
                          bus_name,
                          bus->stops.size(),
                          unique_stops,
                          distance 
        };

        return bus_info;
    }

    TransportCatalogue::StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
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
}