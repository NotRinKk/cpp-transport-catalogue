#include "map_renderer.h"

namespace renderer {
    using namespace domain;
    const Settings& MapRenderer::GetSettings() const {
        return settings_;
    }
    void MapRenderer::Render(std::ostream &out, const SphereProjector proj, std::set<const Bus*, BusComparator> buses, std::set<Stop*, StopComparator> stops) const {
        svg::Document doc;
        AddBusesToMap(doc, proj, buses);
        AddStopsToMap(doc, proj, stops);
        AddStopLabelsToMap(doc, proj, stops);
        doc.Render(out);
    }

    svg::Color MapRenderer::GetNeededColor(std::variant<std::string, std::vector<double>> color) const {
        if (std::holds_alternative<std::string>(color)) {
            return std::get<std::string>(color);
        }
        std::vector<double>& colors = std::get<std::vector<double>>(color);
        if (colors.size() == 3) {
            return GetRgbColor(colors);
        }
        return GetRgbaColor(colors);
    }

    svg::Rgb MapRenderer::GetRgbColor(std::vector<double> color) const {
        return svg::Rgb{static_cast<uint8_t>(color[0]), static_cast<uint8_t>(color[1]), static_cast<uint8_t>(color[2])};
    }
    svg::Rgba MapRenderer::GetRgbaColor(std::vector<double> color) const {
        return svg::Rgba{static_cast<uint8_t>(color[0]), static_cast<uint8_t>(color[1]), static_cast<uint8_t>(color[2]), color[3]};
    }
    svg::Color MapRenderer::GetColor(size_t index) const {
        return GetNeededColor(settings_.color_palette.at(index));
    }
    size_t MapRenderer::GetColorsCount() const {
        return settings_.color_palette.size();
    }
    double MapRenderer::GetLineWidth() const {
        return settings_.line_width;
    }
    svg::Point MapRenderer::GetBusLabelOffset() const {
        return svg::Point{settings_.bus_label_offset[0], settings_.bus_label_offset[1]};
    }

    svg::Point MapRenderer::GetStopLabelOffset() const {
        return svg::Point{settings_.stop_label_offset[0], settings_.stop_label_offset[1]};
    }

    uint32_t  MapRenderer::GetBusLabelFontSize() const {
        return static_cast<uint32_t>(settings_.bus_label_font_size);
    }   

    uint32_t MapRenderer::GetStopLabelFontSize() const {
        return  static_cast<uint32_t>(settings_.stop_label_font_size);
    }

    svg::Color MapRenderer::GetUnderlayerColor() const {
        return GetNeededColor(settings_.underlayer_color);
    }

    double MapRenderer::GetUnderlayerWidth() const {
        return settings_.underlayer_width;
    }

    double MapRenderer::GetStopRadius() const {
        return settings_.stop_radius;
    }

    void MapRenderer::AddStopLabelsToMap(svg::Document& doc, const SphereProjector& proj, std::set<Stop*, StopComparator>& stops) const {
        auto color_stop = GetUnderlayerColor();
        auto stop_label = svg::Text()
                                    .SetOffset(GetStopLabelOffset())
                                    .SetFontSize(GetStopLabelFontSize())
                                    .SetFontFamily("Verdana");
        auto stop_label_base = svg::Text(stop_label)
                                    .SetFillColor(color_stop)
                                    .SetStrokeColor(color_stop)
                                    .SetStrokeWidth(GetUnderlayerWidth())
                                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        auto stop_label_text = svg::Text(stop_label)
                                    .SetFillColor("black");

        for (const auto stop : stops) {
            stop_label_base.SetPosition(proj(stop->coordinates))
                           .SetData(stop->stop_name);
            stop_label_text.SetPosition(proj(stop->coordinates))
                           .SetData(stop->stop_name);
            doc.Add(stop_label_base);
            doc.Add(stop_label_text);
        }
    }

    void MapRenderer::AddStopsToMap(svg::Document& doc, const SphereProjector& proj, std::set<domain::Stop*, domain::StopComparator>& stops) const {
        auto stop_circle = svg::Circle().SetRadius(GetStopRadius())
                                        .SetFillColor("white");
        for (const auto stop : stops) {
            stop_circle.SetCenter(proj(stop->coordinates));
            doc.Add(stop_circle);
        }
    }


    void MapRenderer::AddBusesLines(svg::Document& doc, const SphereProjector& proj, std::set<const domain::Bus*, domain::BusComparator>& buses) const {
        svg::Polyline line;
        size_t count = 0;
        size_t color_number = GetColorsCount();
        for (const auto bus : buses) {
            if(bus->stops.empty()) {
                continue;
            }
            if (count >= color_number) {
                count = 0;
            }
            svg::Color color = GetColor(count);

            auto base_line = svg::Polyline().SetStrokeColor(color).SetFillColor("none").SetStrokeWidth(GetLineWidth())
                                            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            for (size_t stop = 0; stop < bus->stops.size(); ++stop) {
                base_line.AddPoint(proj(bus->stops[stop]->coordinates));
            }

            ++count;
            doc.Add(base_line);
        }
    }

    void MapRenderer::AddBusesLabels(svg::Document& doc, const SphereProjector& proj, std::set<const domain::Bus*, domain::BusComparator>& buses) const {
        size_t count = 0;
        size_t color_number = GetColorsCount();

        for (const auto bus : buses) {
            if(bus->stops.empty()) {
                continue;
            }
            if (count >= color_number) {
                count = 0;
            }    
            svg::Color color = GetColor(count);
            auto label = svg::Text()  
                                    .SetOffset(GetBusLabelOffset())
                                    .SetFontSize(GetBusLabelFontSize())
                                    .SetFontFamily("Verdana")
                                    .SetFontWeight("bold")
                                    .SetData(bus->bus_name); 

            svg::Color underlayer_color = GetUnderlayerColor();  

            auto bus_name_base = svg::Text(label).SetFillColor(underlayer_color)
                                    .SetStrokeColor(underlayer_color)
                                    .SetStrokeWidth(GetUnderlayerWidth())
                                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            auto bus_name_text = svg::Text(label).SetFillColor(color);

            bus_name_base.SetPosition(proj(bus->stops[0]->coordinates));
            bus_name_text.SetPosition(proj(bus->stops[0]->coordinates));
            doc.Add(bus_name_base);
            doc.Add(bus_name_text);
            if (!bus->is_roundtrip) {
                if(bus->stops[bus->stops.size()/2] != bus->stops[0]) {
                    bus_name_base.SetPosition(proj(bus->stops[bus->stops.size()/2]->coordinates));
                    bus_name_text.SetPosition(proj(bus->stops[bus->stops.size()/2]->coordinates));
                    doc.Add(bus_name_base);
                    doc.Add(bus_name_text);
                }
            }
            ++count;
        }
    }
    void MapRenderer::AddBusesToMap(svg::Document& doc, const SphereProjector& proj, std::set<const Bus*, BusComparator>& buses) const {
        AddBusesLines(doc, proj, buses);
        AddBusesLabels(doc, proj, buses);
    }
} // namespace renderer