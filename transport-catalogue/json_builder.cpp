#include "json_builder.h"

namespace json {

    Node Builder::Build() {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            return root_;
        }
        else {
            throw std::logic_error("Wrong JSON structure");
        }
    }

    Builder::KeyItemContext Builder::Key(std::string key) {
        CheckNotBuilt();
        if (!key_expected_) {
            throw std::logic_error("Wrong Key() usage");
        }
        else {
            current_key_ = std::move(key);
            key_expected_ = false;
        }
        return KeyItemContext(*this);
    }       

    Builder& Builder::Value(Node::Value val) {
        CheckNotBuilt();
        AddNode(std::move(Typed(val)));
        return *this;
    }    

    Builder::DictItemContext Builder::StartDict() {
        CheckNotBuilt();
        if (EmptyJson()) {
            root_ = Dict{};
            nodes_stack_.push_back(&root_);
            key_expected_ = true;
        }
        else {
            auto& parent = *nodes_stack_.back();
             if (parent.IsMap() && key_expected_) {
                throw std::logic_error("Cannot add a dictionary inside another dictionary directly as key");    
            }

            AddComplexNode(ComplexType::IsDict);
        }
        return DictItemContext(*this);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        CheckNotBuilt();
        if (EmptyJson()) {
            root_ = Array{};
            nodes_stack_.push_back(&root_);
        }
        else {
            AddComplexNode(ComplexType::IsArray);
        }
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndDict() {
        CheckNotBuilt();
        if (nodes_stack_.back()->IsMap()) {
            nodes_stack_.pop_back();
        }
        else if (nodes_stack_.back()->IsArray()) {
            throw std::logic_error("Wrong container end. Should be Dict");
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        CheckNotBuilt();
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
        }
        else if (nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Wrong container end. Should be Array");
        }
        return *this;
    }

    void Builder::CheckNotBuilt() const {
        if (nodes_stack_.empty() && !root_.IsNull()) {
            throw std::logic_error("Cannot modify already built JSON");
        }
    }

    Node Builder::Typed(Node::Value val) const {
        if (std::holds_alternative<int>(val)) {
            return Node(std::get<int>(val)); 
        } else if (std::holds_alternative<double>(val)) {
            return Node(std::get<double>(val));
        } else if (std::holds_alternative<std::string>(val)) {
            return Node(std::get<std::string>(val)); 
        } else if (std::holds_alternative<json::Array>(val)) {
            return Node(std::get<json::Array>(val));  
        } else if (std::holds_alternative<json::Dict>(val)) {
            return Node(std::get<json::Dict>(val));
        } else if (std::holds_alternative<bool>(val)) {
            return Node(std::get<bool>(val)); 
        }
        return Node(nullptr);
    }

    void Builder::AddComplexNode(ComplexType type) {
        if (nodes_stack_.empty() && root_.IsNull()) {
            if(ComplexType::IsArray) {
                root_ = Array{};
            }
            else {
                root_ = Dict{};
                key_expected_ = true;
            }
            nodes_stack_.push_back(&root_);
        }
        auto& parent = *nodes_stack_.back();
        if (parent.IsArray()) {
            auto& parent_arr = const_cast<Array&>(parent.AsArray());
            if (type == ComplexType::IsArray) {
                parent_arr.emplace_back(Array{}); 
            }
            else if (type == ComplexType::IsDict) {
                parent_arr.emplace_back(Dict{});
                key_expected_ = true;    
            }
            nodes_stack_.push_back(&parent_arr.back());
        }
        else {
            if(key_expected_) {
                throw std::logic_error("Value() must follow Key() inside a dictionary");
            }
            auto& dict = const_cast<Dict&>(parent.AsMap());
            if (type == ComplexType::IsArray) {
                dict[current_key_] = Array{};
                key_expected_ = true;
            }
            else if (type == ComplexType::IsDict) {
                dict[current_key_] = Dict{};
                key_expected_ = true;
            }    
            nodes_stack_.push_back(&dict[current_key_]); 
            
        }
    }

    void Builder::AddNode(Node&& node) {
        if (nodes_stack_.empty() && root_.IsNull()) {
            root_ = std::move(node);
        }
        else {
            auto& parent = *nodes_stack_.back();
            if (parent.IsArray()) {
                auto& parent_arr = const_cast<Array&>(parent.AsArray());
                parent_arr.push_back(std::move(node));                 
            }
            else if (parent.IsMap()) {
                if(key_expected_) {
                    throw std::logic_error("Value() must follow Key() inside a dictionary");
                }
                auto& dict = const_cast<Dict&>(parent.AsMap());
                dict[current_key_] = std::move(node);
                key_expected_ = true;
            }
            else {
                throw std::logic_error("Invalid context for Value()");
            }
                    
        }
    }

    bool Builder::EmptyJson() const {
        return nodes_stack_.empty() && root_.IsNull();
    }
} // namespace json