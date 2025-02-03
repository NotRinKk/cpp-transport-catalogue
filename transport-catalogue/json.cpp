#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    for (char c; input >> c;) {
        if (c == ']') {
            return Node(move(result));
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));

    }
    throw ParsingError("Array parsing error");  
}


std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c;) {
        if (c == '}') {
            return Node(move(result));
        }
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    throw ParsingError("Array parsing error");  
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else  if (std::isdigit(c) || c == '-' || c == '.') {
        input.putback(c);
        auto number = LoadNumber(input);
        if (holds_alternative<int>(number)) {
            return get<int>(number);
        }
        return get<double>(number);
    } else {
        input.putback(c);
        string word;
        while (input.peek() != EOF && isalpha(input.peek())) {
            word += input.get();
        }

        if (word == "null") {
            return Node(nullptr);
        } else if (word == "true") {
            return Node(true);
        } else if (word == "false") {
            return Node(false);
        }
        throw ParsingError("Parsing error. Unexpected token: " + word);    
    }
}


}  // namespace

bool Node::IsInt() const {
    return holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
    return holds_alternative<double>(*this) ||
           holds_alternative<int>(*this);
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(*this);
}

bool Node::IsString() const {
    return holds_alternative<string>(*this);
}

bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(*this);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(*this);
}

int Node::AsInt() const {
    if (IsInt()) {
            return get<int>(*this);
    }
    throw logic_error("Not an int");
}

double Node::AsDouble() const {
    if (IsInt()) {
        return static_cast<double>(get<int>(*this));
    }
    if (IsDouble()) {
        return get<double>(*this);
    }

    throw logic_error("Not a double");
}
const string& Node::AsString() const {
    if (IsString()) {
        return get<string>(*this);
    }
    throw logic_error("Not a string");
}
const Array& Node::AsArray() const {
    if (IsArray()) { 
        return get<Array>(*this);
    }
    throw logic_error("Not an array");
}
bool Node::AsBool() const {
    if(IsBool()) {
        return get<bool>(*this);
    }
    throw logic_error("Not a bool");
}

const Dict& Node::AsMap() const {
    if(IsMap()) {
        return get<Dict>(*this);
    }
    throw logic_error("Not a Map");
}

bool Node::operator==(const Node& other) const {
    return AsVariant() == other.AsVariant();
}
bool Node::operator!=(const Node& other) const {
    return AsVariant() != other.AsVariant();
}

const VariantType& Node::AsVariant() const {
    return static_cast<const VariantType&>(*this);
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}
bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

std::string EscapeString(const std::string& str) {
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

void PrintJson(const Node& node, ostream& out, int indent = 0) {
    const string indented_str(indent, ' ');

    if(node.IsNull()) {
        out << "null";
    } else if (node.IsBool()) {
        out << (node.AsBool() ? "true" : "false");
    } else if (node.IsInt()) {
        out << node.AsInt();
    } else if (node.IsDouble()) {
        out << node.AsDouble();
    } else if(node.IsString()) {
        out << EscapeString(node.AsString());
    } else if (node.IsArray()) {
        out << "[\n";
        const auto& array = node.AsArray();
        for (size_t i = 0; i < array.size(); ++i) {
            out << indented_str << "  ";
            PrintJson(array[i], out, indent + 2);
            if (i + 1 < array.size()) {
                out << ",";
            }
            out << "\n";
        }
        out << indented_str << "]";
    } else if (node.IsMap()) {
        out << "{\n";
        const auto& dict = node.AsMap();
        auto it = dict.begin();
        for (; it != dict.end(); ++it) {
            out << indented_str << "  " << EscapeString(it->first) << ":";
            PrintJson(it->second, out, indent + 2);
            if (std::next(it) != dict.end()) {
                out << ",";
            }
            out << "\n";
        }
        out << indented_str << "}";
    }
    
}

void Print(const Document& doc, std::ostream& output) {
    PrintJson(doc.GetRoot(), output, 0);
}

}  // namespace json