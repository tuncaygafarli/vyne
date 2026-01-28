#include "vcore.h"

void setupBuiltIns(SymbolContainer& env){
    env["global.vcore"] = SymbolTable(); 

    env["global.vcore"]["sub@now"] = Value([](std::vector<Value>& args) -> Value {
        return Value(static_cast<double>(time(0)));
    });
}
