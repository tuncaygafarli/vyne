#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <variant>
#include <sstream>
#include <functional>

class ASTNode;

struct Value {
    enum TypeIndex { NONE = 0, NUMBER = 1, STRING = 2, ARRAY = 3, TABLE = 4, FUNCTION = 5, MODULE = 6 };
    struct FunctionData {
        std::vector<std::string> params;
        std::vector<std::shared_ptr<ASTNode>> body; 

        std::function<Value(std::vector<Value>&)> nativeFn;
        bool isNative = false;
    };

    using VariantData = std::variant<
        std::monostate,
        double,
        std::shared_ptr<std::string>,
        std::shared_ptr<std::vector<Value>>,
        std::unordered_map<std::string, Value>,
        std::shared_ptr<FunctionData>,
        std::string
    >;

    VariantData data;
    bool isReadOnly = false;

    Value() : data(std::monostate{}) {}
    Value(double n) : data(n) {}
    Value(std::string s) 
    : data(std::make_shared<std::string>(std::move(s))) {}
    Value(std::vector<Value> l) 
    : data(std::make_shared<std::vector<Value>>(std::move(l))) {}
    Value(std::shared_ptr<FunctionData> f) : data(std::move(f)) {}
    Value(std::vector<std::string> p, std::vector<std::shared_ptr<ASTNode>> b) {
        auto func = std::make_shared<FunctionData>();
        func->params = std::move(p);
        func->body = std::move(b);
        
        data = std::move(func); 
    }
    Value(std::string moduleName, bool isModule) 
    : data(std::move(moduleName)) {}
    Value(std::function<Value(std::vector<Value>&)> native) {
        auto func = std::make_shared<FunctionData>();
        func->nativeFn = std::move(native);
        func->isNative = true;
        data = std::move(func);
    }
    Value(const Value&) = default;

    // safe getters
    int getType() const { return data.index(); }
    double asNumber() const { return std::get<double>(data); }
    const std::string& asString() const { return *std::get<std::shared_ptr<std::string>>(data); }
    std::vector<Value>& asList() { return *std::get<std::shared_ptr<std::vector<Value>>>(data); }
    const std::vector<Value>& asList() const { return *std::get<std::shared_ptr<std::vector<Value>>>(data); }
    const std::shared_ptr<FunctionData>& asFunction() const { return std::get<std::shared_ptr<FunctionData>>(data); }
    const std::string& asModule() const { return std::get<std::string>(data); }

    // core value functions
    Value& setReadOnly();
    bool isTruthy() const;
    void print(std::ostream& os) const;
    size_t getBytes() const;
    bool equals(const Value& other) const;
    std::string toString() const;
    int toNumber() const;

    bool operator==(const Value& other) const {
        if (data.index() != other.data.index()) return false;

        switch (data.index()) {
            case 0: return true;
            case 1: return std::get<double>(data) == std::get<double>(other.data);
            case 2: return *std::get<std::shared_ptr<std::string>>(data) == *std::get<std::shared_ptr<std::string>>(other.data);
            case 3: return *std::get<std::shared_ptr<std::vector<Value>>>(data) == *std::get<std::shared_ptr<std::vector<Value>>>(other.data);
            default: return false; 
        }
    }

    bool operator<(const Value& other) const {
        if (data.index() != other.data.index()) {
            return data.index() < other.data.index();
        }

        switch (data.index()) {
            case 1:
                return std::get<double>(data) < std::get<double>(other.data);
            case 2:
                return *std::get<std::shared_ptr<std::string>>(data) < *std::get<std::shared_ptr<std::string>>(other.data);
            default:
                return false;
        }
    }

};