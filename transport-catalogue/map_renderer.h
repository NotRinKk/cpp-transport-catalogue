#pragma once
#include <algorithm>
#include <set>
#include <vector>
#include "geo.h"
#include "domain.h"
#include "svg.h"

namespace renderer {
    inline const double EPSILON = 1e-6;
    inline bool IsZeroValue(double value) {
        return std::abs(value) < EPSILON;
    }
    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding) 
        {
            if (points_begin == points_end) {
                return;
            }

            // точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZeroValue(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // коэффициент масштабирования вдоль координаты y
             std::optional<double> height_zoom;
            if (!IsZeroValue(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct Settings {
        double width;
        double height;
        double padding;
        double line_width;
        double stop_radius;
        int bus_label_font_size;
        std::vector<double> bus_label_offset;
        int stop_label_font_size;
        std::vector<double> stop_label_offset;
        std::variant<std::string, std::vector<double>> underlayer_color;
        double underlayer_width;
        std::vector<std::variant<std::string, std::vector<double>>> color_palette;
    };

    class MapRenderer {
    public:
        MapRenderer(Settings settings) 
            : settings_(settings) {
        }

        const Settings& GetSettings() const;
        void Render(std::ostream &out, const SphereProjector proj, std::set<const domain::Bus*, domain::BusComparator> buses, std::set<domain::Stop*, domain::StopComparator> stops) const;
        svg::Color GetNeededColor(std::variant<std::string, std::vector<double>> color) const;
        svg::Rgb GetRgbColor(std::vector<double> color) const;
        svg::Rgba GetRgbaColor(std::vector<double> color) const;
        svg::Color GetColor(size_t index) const;
        size_t GetColorsCount() const;
        double GetLineWidth() const;
        svg::Point GetBusLabelOffset() const;  
        svg::Point GetStopLabelOffset() const; 
        uint32_t GetBusLabelFontSize() const; 
        uint32_t GetStopLabelFontSize() const;   
        svg::Color GetUnderlayerColor() const;
        double GetUnderlayerWidth() const;
        double GetStopRadius() const;
        void AddStopLabelsToMap(svg::Document& doc, const SphereProjector& proj, std::set<domain::Stop*, domain::StopComparator>& stops) const;
        void AddStopsToMap(svg::Document& doc, const SphereProjector& proj, std::set<domain::Stop*, domain::StopComparator>& stops) const;
        void AddBusesLines(svg::Document& doc, const SphereProjector& proj, std::set<const domain::Bus*, domain::BusComparator>& buses) const;
        void AddBusesLabels(svg::Document& doc, const SphereProjector& proj, std::set<const domain::Bus*, domain::BusComparator>& buses) const;
        void AddBusesToMap(svg::Document& doc, const SphereProjector& proj, std::set<const domain::Bus*, domain::BusComparator>& buses) const;

    private:
        Settings settings_;
    };
}