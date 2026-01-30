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

    /**
     * @brief Reads a line of text from the standard input (stdin).
     * * This native function pauses script execution and waits for the user to press Enter.
     * It can optionally take a single string argument to display as a prompt before 
     * reading input.
     * * @param args A vector containing:
     * - args[0] (Optional): A string prompt to display to the user.
     * * @return Value A string containing the user's input. Returns a Null value if
     * input stream fails or reaches end-of-file (EOF).
     * * @note If a non-string argument is provided as the first parameter, it is 
     * ignored and no prompt is displayed.
     */

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

    /**
     * @brief Constrains a numeric value between a minimum and maximum range.
     * * This native function takes three arguments: the value to clamp, the lower bound,
     * and the upper bound. If the value is less than the minimum, the minimum is returned.
     * If the value is greater than the maximum, the maximum is returned.
     * * @param args A vector containing:
     * - args[0]: The input value (Number)
     * - args[1]: The lower bound (Number)
     * - args[2]: The upper bound (Number)
     * * @throw std::runtime_error If the number of arguments is not exactly 3.
     * @return Value The clamped numeric result.
     */

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
    vcore[pool.intern("clamp")]    = Value(VCoreNative::clamp);

    // VCore properties
    vcore[pool.intern("version")]  = Value("v0.0.1-alpha").setReadOnly();
    vcore[pool.intern("engine")]   = Value("Vyne Native").setReadOnly();
    vcore[pool.intern("build")]    = Value(std::string(__DATE__) + " " + std::string(__TIME__)).setReadOnly();
    vcore[pool.intern("cwd")]      = Value(std::filesystem::current_path().string()).setReadOnly();

}