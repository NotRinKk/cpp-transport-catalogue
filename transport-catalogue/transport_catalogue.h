#pragma once
#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <sstream>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "geo.h"
#include "domain.h"

namespace transport_catalogue {
	using namespace domain;
	class TransportCatalogue {

	public:
		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);
		Bus* FindBusByName(std::string_view name) const;
		Stop* FindStopByName(std::string_view name) const;
		BusInfo GetBusInfo(std::string_view bus_name) const;
		StopInfo GetStopInfo(std::string_view stop_name) const;
		void AddAllDistancesToStops(std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> stop_to_distances);
		std::set<const Bus*, BusComparator> GetAllBuses() const;
		std::vector<geo::Coordinates> GetAllStopsCoordinates() const;
		std::set<Stop*, StopComparator> GetAllStopsWithBus() const;

	private:
	    struct StopsHasher {
			size_t operator()(const std::pair<Stop*, Stop*>& p) const {
				size_t hash1 = std::hash<Stop*>()(p.first);
				size_t hash2 = std::hash<Stop*>()(p.second);

				return hash1 * 17 + hash2;
			}
		};
		
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Stop*> index_stops_;
		std::unordered_map<std::string_view, Bus*> index_buses_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stop_;
		std::unordered_map<std::pair<Stop*, Stop*>, int, StopsHasher> stops_distance_;
		
		size_t CountUniqueStopsByBus(Bus* bus) const;
		double  GetBusDistance(Bus* bus) const;
		int GetBusFactDistance(Bus* bus) const;
		void AddStopDistances(std::string_view stop, std::vector<std::pair<int, std::string>> distances_to_stops);
		double CalculateCurvature(Bus* bus) const;
	};
}