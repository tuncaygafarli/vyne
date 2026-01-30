#include "vmem.h"

/**
 * vmem Native Method Implementations
 */

namespace VMemNative {
    Value address(std::vector<Value>& args) {
        if (args.empty()) return Value("0x0");

        const Value& val = args[0];
        const void* actualPtr = nullptr;

        switch (val.getType()) {
            case Value::NUMBER:
                actualPtr = &val.data; 
                break;
            case Value::STRING:
                actualPtr = std::get<std::shared_ptr<std::string>>(val.data).get();
                break;
            case Value::ARRAY:
                actualPtr = std::get<std::shared_ptr<std::vector<Value>>>(val.data).get();
                break;
            case Value::FUNCTION:
                actualPtr = val.asFunction().get();
                break;
            default:
                actualPtr = &val.data;
        }

        std::stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<uintptr_t>(actualPtr);
        return Value(ss.str());
    }
}

void setupVMem(SymbolContainer& env, StringPool& pool) {
    std::string path = "global.vmem";
    
    if (env.find(path) == env.end()) {
        env[path] = SymbolTable();
    }

    auto& vmem = env[path];

    // vmem methods
    vmem[pool.intern("address")] = Value(VMemNative::address);

    // vmem properties
}