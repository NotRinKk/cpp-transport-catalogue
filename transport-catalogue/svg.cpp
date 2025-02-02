#include "svg.h"

namespace svg {

using namespace std::literals;


std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
    switch(line_cap) {
        case StrokeLineCap::BUTT: os << "butt"; break;
        case StrokeLineCap::ROUND: os << "round"; break;
        case StrokeLineCap::SQUARE: os << "square"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
    switch(line_join) {
        case StrokeLineJoin::ARCS: os << "arcs"; break;
        case StrokeLineJoin::BEVEL: os << "bevel"; break;
        case StrokeLineJoin::MITER: os << "miter"; break;
        case StrokeLineJoin::MITER_CLIP: os << "miter-clip"; break;
        case StrokeLineJoin::ROUND: os << "round"; break;
    }
    return os;
}

std::string EscapeSpecialChars(const std::string& text) {
    std::ostringstream escaped;

    for (char c : text) {
        switch (c) {
            case '"': escaped << "&quot;"; break;
            case '\'': escaped << "&apos;"; break;
            case '<': escaped << "&lt;"; break;
            case '>': escaped << "&gt;"; break;
            case '&': escaped << "&amp;"; break;
            default: escaped << c; break;
        }
    }
    return escaped.str();
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}


Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x <<"\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_ << "\""sv;
    RenderAttrs(context.out);
    if (font_family_) {
        out <<  " font-family=\""sv << *font_family_ <<"\"";
    }
    if (font_weight_) {
        out <<  " font-weight=\""sv << *font_weight_ <<"\"";
    }
    
    out << ">"sv;
    out << EscapeSpecialChars(data_);
    out << "</text>"sv;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    bool is_first = true;
    out << "<polyline points=\""sv;
    for (const Point p : points_) {
        if (is_first) {
            out << p.x << ","sv << p.y;
            is_first = false;
        }
        else {
            out << " " << p.x << ","sv << p.y;
        }
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << " />";
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for (auto& obj : objects_) {
        out << "  ";
        obj->Render(out);
    }

    out << "</svg>";
}
}  // namespace svg