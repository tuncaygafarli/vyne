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
#include <cstdint>

class ASTNode;
struct ModuleData { 
    uint32_t moduleId;
    std::string name; 
};

struct Value {
    enum TypeIndex { 
        NONE = 0, 
        NUMBER = 1, 
        STRING = 2, 
        ARRAY = 3, 
        FUNCTION = 4, 
        MODULE = 5 
    };
    struct FunctionData {
        std::vector<uint32_t> params;
        std::vector<std::shared_ptr<ASTNode>> body; 

        std::function<Value(std::vector<Value>&)> nativeFn;
        bool isNative = false;
    };

    using VariantData = std::variant<
        std::monostate,
        double,
        std::shared_ptr<std::string>,
        std::shared_ptr<std::vector<Value>>,
        std::shared_ptr<FunctionData>,
        ModuleData
    >;

    VariantData data;
    bool isReadOnly = false;

    // constructors
    Value() : data(std::monostate{}) {}
    Value(double n) : data(n) {}
    Value(std::string s) 
    : data(std::make_shared<std::string>(std::move(s))) {}
    Value(std::vector<Value> l) 
    : data(std::make_shared<std::vector<Value>>(std::move(l))) {}
    Value(std::shared_ptr<FunctionData> f) : data(std::move(f)) {}
    Value(std::vector<uint32_t> p, std::vector<std::shared_ptr<ASTNode>> b) {
        auto func = std::make_shared<FunctionData>();
        func->params = std::move(p);
        func->body = std::move(b);
        
        data = std::move(func); 
    }
    Value(uint32_t mId, std::string moduleName, bool isModule) 
        : data(ModuleData{mId, std::move(moduleName)}) {}
    Value(std::function<Value(std::vector<Value>&)> native) {
        auto func = std::make_shared<FunctionData>();
        func->nativeFn = std::move(native);
        func->isNative = true;
        data = std::move(func);
    }
    Value(const Value&) = default;

    // safe getters
    int getType() const { return static_cast<int>(data.index()); }
    std::string getTypeName() const { 
        int type = getType();
        switch(type) {
            case Value::NUMBER:   return "Number";
            case Value::STRING:   return "String";
            case Value::ARRAY:    return "Array";
            case Value::FUNCTION: return "Function";
            case Value::MODULE:   return "Module";
            default:              return "Unknown";
        }
    }

    double asNumber() const { 
        return std::get<double>(this->data); 
    }

    const std::string& asString() const { 
        return *std::get<std::shared_ptr<std::string>>(this->data); 
    }

    std::vector<Value>& asList() { 
        return *std::get<std::shared_ptr<std::vector<Value>>>(this->data); 
    }

    const std::vector<Value>& asList() const { 
        return *std::get<std::shared_ptr<std::vector<Value>>>(this->data); 
    }

    const std::shared_ptr<FunctionData>& asFunction() const { 
        return std::get<std::shared_ptr<FunctionData>>(this->data); 
    }

    const std::string& asModule() const { 
        return std::get<ModuleData>(data).name;
    }

    // core value functions
    Value& setReadOnly();
    bool isTruthy() const;
    void print(std::ostream& os) const;
    size_t getDeepBytes() const;
    size_t getShallowBytes() const;
    bool equals(const Value& other) const;
    std::string toString() const;
    int toNumber() const;

    bool operator==(const Value& other) const {
        if (this->data.index() != other.data.index()) return false;

        switch (this->data.index()) {
            case 0: return true;
            case 1: return std::get<double>(this->data) == std::get<double>(other.data);
            case 2: return *std::get<std::shared_ptr<std::string>>(this->data) == *std::get<std::shared_ptr<std::string>>(other.data);
            case 3: return *std::get<std::shared_ptr<std::vector<Value>>>(this->data) == *std::get<std::shared_ptr<std::vector<Value>>>(other.data);
            default: return false; 
        }
    }

    bool operator!=(const Value& other) const {
        if (this->data.index() != other.data.index()) return false;

        switch (this->data.index()) {
            case 0: return false;
            case 1: return std::get<double>(this->data) != std::get<double>(other.data);
            case 2: return *std::get<std::shared_ptr<std::string>>(this->data) != *std::get<std::shared_ptr<std::string>>(other.data);
            case 3: return *std::get<std::shared_ptr<std::vector<Value>>>(this->data) != *std::get<std::shared_ptr<std::vector<Value>>>(other.data);
            default: return true; 
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

// TODO ADD POOL CLEARING FEATURE WHEN THE DISMISS IS TRIGGERED

class StringPool {
    std::vector<std::string> idToStr;
    std::unordered_map<std::string, uint32_t> strToId;

public:
    static StringPool& instance() {
        static StringPool pool;
        return pool;
    }
    
    static uint32_t intern(const std::string& s);

    const std::string& get(uint32_t id) { return idToStr[id]; }
};