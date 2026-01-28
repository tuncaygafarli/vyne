#include "vcore.h"
#include <ctime>
#include <chrono>
#include <thread>
#include <random>
#include <iostream>

/**
 * VCore Native Method Implementations
 */
namespace VCoreNative {

    Value now(std::vector<Value>& args) {
        return Value(static_cast<double>(std::time(0)));
    }

    Value sleep(std::vector<Value>& args) {
        if (args.empty()) throw std::runtime_error("vcore.sleep() expects 1 argument (ms)");
        long long ms = static_cast<long long>(args[0].asNumber());
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return Value(true);
    }

    Value platform(std::vector<Value>& args) {
        #ifdef _WIN32
            return Value("Windows 32-bit");
        #elif _WIN64
            return Value("Windows 64-bit");
        #elif __APPLE__ || __MACH__
            return Value("Mac OSX");
        #elif __linux__
            return Value("Linux");
        #elif __FreeBSD__
            return Value("FreeBSD");
        #elif __unix || __unix__
            return Value("Unix");
        #else
            return Value("Other/Unknown");
        #endif
    }

    Value random(std::vector<Value>& args) {
        if (args.size() < 2) throw std::runtime_error("Argument Error : vcore.random() expects 2 arguments (min, max)");
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(
            static_cast<int>(args[0].asNumber()), 
            static_cast<int>(args[1].asNumber())
        );
        return Value(static_cast<double>(dist(gen)));
    }
}


void setupBuiltIns(SymbolContainer& env) {
    if (env.find("global.vcore") == env.end()) {
        env["global.vcore"] = SymbolTable();
    }

    auto& vcore = env["global.vcore"];

    // VCore methods
    vcore["sub@now"]      = Value(VCoreNative::now);
    vcore["sub@sleep"]    = Value(VCoreNative::sleep);
    vcore["sub@platform"] = Value(VCoreNative::platform);
    vcore["sub@random"]   = Value(VCoreNative::random);

    // VCore properties
    vcore["version"]      = Value("0.0.1-alpha");
}