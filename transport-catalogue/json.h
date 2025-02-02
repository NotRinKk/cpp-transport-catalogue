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

    using Number = std::variant<int, double>;



    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t) : value_(nullptr) {}
        Node(bool value) : value_(value) {}
        Node(int value) : value_(value) {}
        Node(double value) : value_(value) {}
        Node(const std::string& value) : value_(value) {}
        Node(Array array) : value_(std::move(array)) {}
        Node(Dict map) : value_(std::move(map)) {}

        const Value& GetValue() const;
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

    private:
        Value value_;

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