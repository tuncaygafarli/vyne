#ifndef VYNE_VM_H
#define VYNE_VM_H

#include "../compiler/ast/ast.h"
#include "../compiler/codegen/chunk.h"
#include <vector>

enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
    Chunk* chunk;
    uint8_t* ip;
    std::vector<Value> stack;
    SymbolContainer& globals; 
    std::string currentGroup = "global";

public:
    VM(SymbolContainer& env) : chunk(nullptr), ip(nullptr), globals(env) {}
    InterpretResult interpret(Chunk& chunk);
    InterpretResult run();
    
    void push(Value value) { stack.push_back(value); }
    Value pop() { 
        Value val = stack.back(); 
        stack.pop_back(); 
        return val; 
    }
};

#endif