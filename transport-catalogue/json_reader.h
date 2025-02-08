#pragma once
#include <algorithm>
#include <istream>
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
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

    private:

        std::string EscapeString(const std::string& str) const;
        std::variant<std::string, std::vector<double>> GetColor(const json::Node& value);
        std::vector<std::variant<std::string, std::vector<double>>> GetColorPalette(const json::Array& array);
        std::vector<std::string_view> GetRoute(const json::Array& stops, bool is_roundtrip);
        void AddBuses(const json::Array& array);
        void AddStops(const json::Array& array);
        void AddAllDistances(std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> distances);

        json::Document doc_;
        transport_catalogue::TransportCatalogue catalogue_;
    };
}