#include "vmath.h"

/**
 * VMathNative Native Method Implementations
 */

namespace VMathNative {
    Value clamp(std::vector<Value>& args) {
        if (args.size() != 3) {
            throw std::runtime_error("Argument Error: vmath.clamp() expects 3 arguments (val, min, max), but got " + std::to_string(args.size()) + ".");
        }

        double val = args[0].asNumber();
        double min = args[1].asNumber();
        double max = args[2].asNumber();

        if (min > max) std::swap(min, max);

        if (val < min) return Value(min);
        if (val > max) return Value(max);
        
        return Value(val);
    }

    Value sqrt(std::vector<Value>& args) {
        if (args.size() != 1) {
            throw std::runtime_error("Argument Error: vmath.sqrt() expects 1 argument, but got " + std::to_string(args.size()) + ".");
        }

        if (args[0].getType() != Value::NUMBER) {
            throw std::runtime_error("Argument Error: vmath.sqrt() excepts only Number type as argument, but got " + args[0].getTypeName());
        }

        double val = args[0].asNumber();

        if (val < 0) {
            throw std::runtime_error("Runtime Error: Square root of negative number is not supported in vmath.sqrt().");
        }

        double returnVal = std::sqrt(val);

        return Value(returnVal);
    }

    Value abs(std::vector<Value>& args) {
        if (args.size() != 1) {
            throw std::runtime_error("Argument Error: vmath.abs() expects 1 argument, but got " + std::to_string(args.size()) + ".");
        }

        if (args[0].getType() != Value::NUMBER) {
            throw std::runtime_error("Argument Error: vmath.abs() expects only Number type as argument, but got " + args[0].getTypeName());
        }

        double val = args[0].asNumber();
        
        double returnVal = std::abs(val);

        return Value(returnVal);
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
    vmath[pool.intern("sqrt")]     = Value(VMathNative::sqrt);
    vmath[pool.intern("abs")]      = Value(VMathNative::abs);

    // VMath constants
    vmath[pool.intern("pi")]       = Value(3.141592653589793).setReadOnly();
    vmath[pool.intern("e")]        = Value(2.718281828459045).setReadOnly();
    vmath[pool.intern("tau")]      = Value(6.283185307179586).setReadOnly();
    vmath[pool.intern("phi")]      = Value(1.618033988749895).setReadOnly();
    vmath[pool.intern("sqrt2")]    = Value(1.414213562373095).setReadOnly();

    // VMath properties
    vmath[pool.intern("version")]  = Value("v0.0.1-alpha").setReadOnly();
}