#ifndef VYNE_CHUNK_H
#define VYNE_CHUNK_H

#include <vector>
#include <cstdint>
#include "../ast/value.h"

enum OpCode : uint8_t {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
};

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines;

    void write(uint8_t byte, int line) {
        code.push_back(byte);
        lines.push_back(line);
    }

    int addConstant(Value value) {
        constants.push_back(value);
        return constants.size() - 1;
    }
};

#endif

void disassembleChunk(const Chunk& chunk, const std::string& name);