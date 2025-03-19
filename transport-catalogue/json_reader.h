#pragma once
#include <algorithm>
#include <istream>
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace json_reader {
    class JsonReader {
    public:
        explicit JsonReader(std::istream& input);
        void BuildBase();
        const transport_catalogue::TransportCatalogue& GetCatalogue() const;
        void PrintResponse(request_handler::RequestHandler& handler);
        renderer::Settings GetRenderSettings();

        router::Settings GetRoutingSettings();

    private:
        std::string EscapeString(const std::string& str) const;
        std::variant<std::string, std::vector<double>> GetColor(const json::Node& value);
        std::vector<std::variant<std::string, std::vector<double>>> GetColorPalette(const json::Array& array);
        std::vector<std::string_view> GetRoute(const json::Array& stops, bool is_roundtrip);
        void AddBuses(const json::Array& array);
        void AddStops(const json::Array& array);
        void AddAllDistances(std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> distances);
        void HandleStopRequest(const json::Dict& command_data, request_handler::RequestHandler& handler, json::Builder& builder) const;
        void HandleBusRequest(const json::Dict& command_data, request_handler::RequestHandler& handler, json::Builder& builder) const;
        void HandleMapRequest(const json::Dict& command_data, request_handler::RequestHandler& handler, json::Builder& builder) const;
        void HandleRouteRequest(const json::Dict& command_data, request_handler::RequestHandler& handler, json::Builder& builder) const;

        json::Document doc_;
        transport_catalogue::TransportCatalogue catalogue_;
    };
} // namespace json_reader