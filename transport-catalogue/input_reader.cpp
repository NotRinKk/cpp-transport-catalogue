#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace input_reader {
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return {nan, nan};
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return {lat, lng};
    }

    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1))};
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue)  {
        auto command_comp =  [](const CommandDescription& lhs, const CommandDescription& rhs) {
        return lhs.command > rhs.command;
        };

        std::sort(commands_.begin(), commands_.end(), command_comp);

        for (const CommandDescription& cmd : commands_) {
            if (Trim(cmd.command) == "Stop") {
                transport_catalogue::TransportCatalogue::Stop stop = { std::string(Trim(cmd.id)),
                                                ParseCoordinates(Trim(cmd.description)) };        
                
                catalogue.AddStop(std::move(stop));
            }
            else {
                std::vector<transport_catalogue::TransportCatalogue::Stop*> bus_stops;
                std::vector<std::string_view> bus_stops_names = ParseRoute(Trim(cmd.description));

                for (const auto stop_name : bus_stops_names) {
                    bus_stops.push_back(catalogue.FindStopByName(stop_name));
                }

                transport_catalogue::TransportCatalogue::Bus bus = { std::string(Trim(cmd.id)),
                                                bus_stops }; 

                catalogue.AddBus(std::move(bus));
            }
        }
    }
}