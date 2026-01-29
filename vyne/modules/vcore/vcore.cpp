#include "vcore.h"

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

    Value string(std::vector<Value>& args){
        if (args.size() != 1) throw std::runtime_error("Argument Error : vcore.string() expects 1 argument, but got " + std::to_string(args.size()) + " instead.");

        return Value(args[0].toString());
    }

    Value number(std::vector<Value>& args){
        if (args.size() != 1) throw std::runtime_error("Argument Error : vcore.number() expects 1 argument, but got " + std::to_string(args.size()) + " instead.");

        return Value(args[0].toNumber());
    }

    Value input(std::vector<Value>& args){
        if(!args.empty() && args[0].getType() == Value::STRING){
            std::cout << args[0].asString();
        }

        std::string input;
        if(std::getline(std::cin, input)){
            return Value(input);
        }

        return Value();
    }
}

void setupVCore(SymbolContainer& env, StringPool& pool) {
    std::string path = "global.vcore";
    
    if (env.find(path) == env.end()) {
        env[path] = SymbolTable();
    }

    auto& vcore = env[path];


    // VCore methods
    vcore[pool.intern("now")]      = Value(VCoreNative::now);
    vcore[pool.intern("sleep")]    = Value(VCoreNative::sleep);
    vcore[pool.intern("platform")] = Value(VCoreNative::platform);
    vcore[pool.intern("random")]   = Value(VCoreNative::random);
    vcore[pool.intern("string")]   = Value(VCoreNative::string);
    vcore[pool.intern("number")]   = Value(VCoreNative::number);
    vcore[pool.intern("input")]    = Value(VCoreNative::input);

    // VCore properties
    vcore[pool.intern("version")]  = Value("v0.0.1-alpha").setReadOnly();
}