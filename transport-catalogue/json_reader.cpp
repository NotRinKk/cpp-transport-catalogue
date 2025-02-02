#include "json_reader.h"

namespace json_reader {
    JsonReader::JsonReader(std::istream& input) 
        : doc_(json::Load(input))
        , catalogue_(transport_catalogue::TransportCatalogue()) {
    }

    void JsonReader::BuildBase() {
        const auto& db_req_array = doc_.GetRoot().AsMap().at("base_requests").AsArray();
        AddStops(db_req_array);
        AddBuses(db_req_array);
    }

    const transport_catalogue::TransportCatalogue& JsonReader::GetCatalogue() const {
        return catalogue_;
    }

    std::string JsonReader::EscapeString(const std::string& str) const {
        std::string result = "\"";
        for (char ch : str) {
            switch (ch) {
                case '\"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += ch; break;
            }
        }
        result += "\"";
        return result;
    }
    void JsonReader::PrintResponse(request_handler::RequestHandler& handler) {
        std::cout << "[" << std::endl;
        std::string space(4, ' ');
        auto requests = doc_.GetRoot().AsMap().at("stat_requests").AsArray();
        bool is_first_req = true;
        for (const auto& req : requests) {
            if (!is_first_req) {
                std::cout << "," << std::endl;
            }
            is_first_req = false;
            auto command = req.AsMap();
            std::cout << space << "{" << std::endl;

            if (command.at("type").AsString() == "Stop") {
                auto response = handler.GetBusesByStop(command.at("name").AsString());
                if (response) {
                    std::cout << space << space << "\"buses\": [" << std::endl;
                    bool is_first = true;
                    auto buses = response.value();
                    for (const auto& element : buses) {
                        if (!is_first) {
                            std::cout << ", " << std::endl;
                        }
                        is_first = false;
                        std::cout << space << space << space << "\"" << element << "\"";
                    }
                    std::cout << std::endl << space << space <<  "]," << std::endl;
                    std::cout << space << space << "\"request_id\": " << command.at("id").AsInt() << std::endl;
                }
                else {
                    std::cout << space << space << "\"request_id\": " << command.at("id").AsInt() << "," << std::endl;
                    std::cout << space << space << "\"error_message\": \"not found\"" << std::endl;
                }
            }
            else if (command.at("type").AsString() == "Bus") {
                
                auto response = handler.GetBusStat(command.at("name").AsString());
                if (response) {
                    std::cout << space << space << "\"curvature\": " << response.value().curvature << "," << std::endl;
                    std::cout << space << space << "\"request_id\": " << command.at("id").AsInt() << "," << std::endl;
                    std::cout << space << space << "\"route_length\": " << response.value().distance << "," << std::endl;
                    std::cout << space << space << "\"stop_count\": " << response.value().stops << "," << std::endl;
                    std::cout << space << space << "\"unique_stop_count\": " << response.value().unique_stops << std::endl;
                }
                else {
                    std::cout << space << space << "\"request_id\": " << command.at("id").AsInt() << "," << std::endl;
                    std::cout << space << space << "\"error_message\": \"not found\"" << std::endl;
                }
            }
            else {
                std::ostringstream svg_output;
                handler.RenderMap(svg_output);

                std::string map = svg_output.str();
                map = std::move(EscapeString(map));
                std::cout << space << space << "\"map\": " << map << "," << std::endl;
                std::cout << space << space << "\"request_id\": " << command.at("id").AsInt() << std::endl;
            }
            std::cout << space << "}";
        }
        std::cout << std::endl << "]";
    }

    std::vector<std::string_view> JsonReader::GetRoute(const json::Array& stops, bool is_roundtrip) {
        std::vector<std::string_view> route;
            for (auto& name : stops) {
                route.push_back(name.AsString());
            }
            if (!is_roundtrip) {
                for (auto it = std::next(stops.rbegin()); it != stops.rend(); ++it) {
                    route.push_back((*it).AsString());
                }
            }

        return route;
    }

    void JsonReader::AddBuses(const json::Array& array) {
        for (auto& dict : array) {
            if ( auto element = dict.AsMap(); element.at("type").AsString() == "Bus") {
                std::vector<std::string_view> bus_route = GetRoute(element.at("stops").AsArray(),
                                                                   element.at("is_roundtrip").AsBool());
                
                std::vector<domain::Stop*> bus_stops;
                for (const auto stops_name : bus_route) {
                    bus_stops.push_back(catalogue_.FindStopByName(stops_name));
                }

                domain::Bus bus = {element.at("is_roundtrip").AsBool(), element.at("name").AsString(), bus_stops};
                catalogue_.AddBus(std::move(bus));
            }
        }
    }

    void JsonReader::AddStops(const json::Array& array) {
        std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> stops_to_distances;
        for (auto& dict : array) {
            if ( auto element = dict.AsMap(); element.at("type").AsString() == "Stop") {
                domain::Stop stop = {element.at("name").AsString(), 
                                     geo::Coordinates{element.at("latitude").AsDouble(),
                                                      element.at("longitude").AsDouble()}};

                std::vector<std::pair<int, std::string>> distances;                            
                for (auto& stop_dist : element.at("road_distances").AsMap()) {
                    distances.push_back(std::make_pair(stop_dist.second.AsInt(), stop_dist.first));
                }
                if(!distances.empty()) {
                    stops_to_distances[stop.stop_name] = std::move(distances);
                }
                catalogue_.AddStop(std::move(stop));
            }
        }
        AddAllDistances(std::move(stops_to_distances));
    }

    void JsonReader::AddAllDistances(std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> distances) {
        catalogue_.AddAllDistancesToStops(std::move(distances));
    }

    renderer::Settings JsonReader::GetRenderSettings() {
        renderer::Settings settings;
        const json::Dict& dict =  doc_.GetRoot().AsMap().at("render_settings").AsMap();
        settings.width = dict.at("width").AsDouble();
        settings.height = dict.at("height").AsDouble();
        settings.padding = dict.at("padding").AsDouble();
        settings.line_width = dict.at("line_width").AsDouble();
        settings.stop_radius = dict.at("stop_radius").AsDouble();
        settings.bus_label_font_size = dict.at("bus_label_font_size").AsInt();
        settings.bus_label_offset = std::vector{dict.at("bus_label_offset").AsArray()[0].AsDouble(),
                                                dict.at("bus_label_offset").AsArray()[1].AsDouble()};
        settings.stop_label_font_size = dict.at("stop_label_font_size").AsInt();
        settings.stop_label_offset = std::vector{dict.at("stop_label_offset").AsArray()[0].AsDouble(),
                                                 dict.at("stop_label_offset").AsArray()[1].AsDouble()};
        settings.underlayer_color = GetColor(dict.at("underlayer_color"));
                                              
        settings.underlayer_width = dict.at("underlayer_width").AsDouble();
        settings.color_palette = std::move(GetColorPalette(dict.at("color_palette").AsArray()));
        return settings;
    }

    std::variant<std::string, std::vector<double>> JsonReader::GetColor(const json::Node& value) {
        std::variant<std::string, std::vector<double>> color;
        if (value.IsArray()) {
            std::vector<double> rgb;
            if (auto& arr = value.AsArray(); arr.size() == 3) {
                for (auto& element : arr) { 
                    rgb.push_back(element.AsInt());
                }
                color = std::move(rgb);
            }
            else {
                std::vector<double> rgba;
                for (auto& element : arr) { 
                    rgba.push_back(element.AsDouble());
                }
                color = std::move(rgba);
            }
        }
        else if (value.IsString()) {
            color = value.AsString();
        }
        return color;
    }

    std::vector<std::variant<std::string, std::vector<double>>> JsonReader::GetColorPalette(const json::Array& array) {
        std::vector<std::variant<std::string, std::vector<double>>> colors;
        for (auto& color : array) {
            colors.push_back(GetColor(color));
        }
        return colors;
    }
}