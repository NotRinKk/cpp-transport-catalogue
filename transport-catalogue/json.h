#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    using VariantType = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    class Node final
    : private VariantType
    {
    public:
        using variant::variant;
        using Value = variant;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;
        const Value& GetValue() const;
    
    private:
        const VariantType& AsVariant() const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json