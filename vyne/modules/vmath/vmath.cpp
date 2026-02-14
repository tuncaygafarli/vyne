#include "vmath.h"

/**
 * VMathNative Native Method Implementations
 */

namespace VMathNative {
    Value clamp(std::vector<Value>& args) {
        if (args.size() != 3) {
            throw std::runtime_error("Argument Error: vcore.clamp() expects 3 arguments (val, min, max), but got " + std::to_string(args.size()) + ".");
        }

        double val = args[0].asNumber();
        double min = args[1].asNumber();
        double max = args[2].asNumber();

        if (min > max) std::swap(min, max);

        if (val < min) return Value(min);
        if (val > max) return Value(max);
        
        return Value(val);
    }
}

void setupVMath(SymbolContainer& env, StringPool& pool) {
    std::string path = "global.vmath";
    
    if (env.find(path) == env.end()) {
        env[path] = SymbolTable();
    }

    auto& vmath = env[path];

    // VMath methods
    vmath[pool.intern("clamp")]    = Value(VMathNative::clamp);

    // VMath properties
    vmath[pool.intern("version")]  = Value("v0.0.1-alpha").setReadOnly();
}