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

namespace transport_catalogue {
	class TransportCatalogue {
	public:
		struct Stop	{
			std::string stop_name;
			geo::Coordinates coordinates;
		};

		struct Bus {
			std::string bus_name;
			std::vector<Stop*> stops;
		};
		
		struct BusInfo {
			bool exists;
			std::string_view name;
			size_t stops;
			size_t unique_stops;
			double distance;
		};

		struct StopInfo {
			bool exists;
			std::string_view name;
			std::set<std::string_view> buses;
		};

		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);
		Bus* FindBusByName(std::string_view name) const;
		Stop* FindStopByName(std::string_view name) const;
		BusInfo GetBusInfo(std::string_view bus_name) const;
		StopInfo GetStopInfo(std::string_view stop_name) const;

	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Stop*> index_stops_;
		std::unordered_map<std::string_view, Bus*> index_buses_;
		std::unordered_map<std::string_view, std::set<std::string_view>> buses_to_stop_;

		size_t CountUniqueStopsByBus(TransportCatalogue::Bus* bus) const;
		double  GetBusDistance(TransportCatalogue::Bus* bus) const;
	};
}