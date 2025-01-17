#include "stat_reader.h"

namespace stat_reader {
    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                        std::ostream& output) {

        auto start_pos = request.find_first_not_of(' ');
        auto end_pos = request.find_first_of(' ', start_pos);

        std::string_view command = request.substr(start_pos, end_pos - start_pos);

        std::string_view info_of = request.substr(request.find_first_not_of(' ', end_pos));
        
        if (command == "Bus") {

            const auto bus_info = transport_catalogue.GetBusInfo(info_of);
            
            if (!bus_info.exists) {
                output << "Bus " << bus_info.name << ": not found";      
            }
            else {
                output << "Bus " << bus_info.name << ": " 
                    << bus_info.stops << " stops on route, " 
                    << bus_info.unique_stops << " unique stops, "
                    << std::setprecision(6) << bus_info.distance << " route length, "
                    << bus_info.curvature << " curvature";
            }
        }
        else if (command == "Stop") {

            const auto stop_info = transport_catalogue.GetStopInfo(info_of);

            if(!stop_info.exists) {
                output << "Stop " << stop_info.name << ": not found";
            }
            else if(stop_info.buses.empty()) {
                output << "Stop " << stop_info.name << ": no buses";
            }
            else {
                output << "Stop " << stop_info.name << ": buses";
                for (const auto bus : stop_info.buses) {
                    output << " " << bus;
                }
            }
        }
        output << std::endl;

    }
}