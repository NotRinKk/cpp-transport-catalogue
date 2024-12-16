#pragma once

#include <deque>
#include <iomanip>
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
		
		void AddStop(const Stop& stop);
		void AddBus(const Bus& bus);
		Bus* FindBusByName(std::string_view name) const;
		Stop* FindStopByName(std::string_view name) const;
		std::string GetBusInfo(std::string_view bus_name) const;
		std::string GetStopInfo(std::string_view stop_name) const;

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