#pragma once

#include "json.h"

namespace json {
    
    class Builder {
    private:
        class BaseContext;
        class DictItemContext;
        class KeyItemContext;
        class ArrayItemContext;
    public:
        Builder() = default;

        Node Build();
        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value val);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::string current_key_;
        bool key_expected_ = false;
        
        enum ComplexType {
            None,
            IsArray,
            IsDict
        };

        void CheckNotBuilt() const;
        Node Typed(Node::Value val) const;
        void AddComplexNode(ComplexType type);
        void AddNode(Node&& node);
        bool EmptyJson() const;
        
        class BaseContext {
            public:
                BaseContext(Builder& builder) : builder_(builder) {}
                Node Build() {
                    return builder_.Build();
                }
                KeyItemContext Key(std::string key) {
                    return builder_.Key(std::move(key));
                }
                BaseContext Value(Node::Value value) {
                    return builder_.Value(std::move(value));
                }
                DictItemContext StartDict() {
                    return builder_.StartDict();
                }
                ArrayItemContext StartArray() {
                    return builder_.StartArray();
                }
                BaseContext EndDict() {
                    return builder_.EndDict();
                }
                BaseContext EndArray() {
                    return builder_.EndArray();
                }
            private:
                Builder& builder_;
        };

        class DictItemContext : public BaseContext {
        public:
            DictItemContext(BaseContext base) : BaseContext(base) {}
            Node Build() = delete;
            BaseContext Value(Node::Value value) = delete;
            BaseContext EndArray() = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
        };
        
        class KeyItemContext : public BaseContext {
        public:
            KeyItemContext(BaseContext base) : BaseContext(base) {}
            Node Build() = delete;
            DictItemContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
            DictItemContext Value(Node::Value value) { return BaseContext::Value(std::move(value)); }
        };
        
        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(BaseContext base) : BaseContext(base) {}
            ArrayItemContext Value(Node::Value value) { return BaseContext::Value(std::move(value)); }
            Node Build() = delete;
            KeyItemContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
        };
        
    
    };
}