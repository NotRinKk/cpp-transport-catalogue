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
        if (index_buses_.count(name) == 0) {
            return nullptr;
        }
        return index_buses_.at(name);
    }

    TransportCatalogue::Stop* TransportCatalogue::FindStopByName(std::string_view name) const {
        if(index_stops_.count(name) == 0) {
            return nullptr;
        }
        return index_stops_.at(name);
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

    std::string TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        using namespace std;

        string info;
        auto bus = FindBusByName(bus_name);
        if (bus == nullptr) {
            info = "Bus " + string(bus_name) + ": not found\n";
            return info;
        }

        size_t unique_stops = CountUniqueStopsByBus(bus);
        double distance = GetBusDistance(bus);

        ostringstream oss;
        oss << "Bus " << bus_name << ": " 
            << bus->stops.size() << " stops on route, " 
            << unique_stops << " unique stops, "
            << setprecision(6) << distance << " route length" << endl;

        info = oss.str();

        return info;
    }

    std::string TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
        using namespace std;

        string info = "Stop " + string(stop_name);
        auto stop = FindStopByName(stop_name);
        if (stop == nullptr) {
            info += ": not found\n";
            return info;
        }

        const auto has_buses = buses_to_stop_.count(stop_name);
        if (has_buses == 0) {
            info += ": no buses\n";
            return info;
        }

        bool is_first = true;
        for (const auto bus_name : buses_to_stop_.at(stop_name)) {
            if (is_first) {
                info += ": buses " + string(bus_name);
                is_first = false;
            }
            else {
                info += " " + string(bus_name);
            }

        }
        info += "\n";

        return info;
    }
}