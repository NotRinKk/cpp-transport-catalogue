#include "stat_reader.h"

namespace stat_reader {
    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                        std::ostream& output) {

        auto start_pos = request.find_first_not_of(' ');
        auto end_pos = request.find_first_of(' ', start_pos);

        std::string_view command = request.substr(start_pos, end_pos - start_pos);

        std::string_view info_of = request.substr(request.find_first_not_of(' ', end_pos));
        
        if (command == "Bus") {

            output << transport_catalogue.GetBusInfo(info_of);
        }
        else if (command == "Stop") {

            output << transport_catalogue.GetStopInfo(info_of);
        }

    }
}