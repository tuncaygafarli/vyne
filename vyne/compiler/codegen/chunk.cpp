#include "chunk.h"

void disassembleChunk(const Chunk& chunk, const std::string& name) {
    std::cout << "== " << name << " ==" << std::endl;

    for (int offset = 0; offset < (int)chunk.code.size(); ) {
        uint8_t instruction = chunk.code[offset];
        std::printf("%04d ", offset);

        switch (instruction) {
            case OP_CONSTANT: {
                uint8_t constantIndex = chunk.code[offset + 1];
                std::printf("OP_CONSTANT %d '", constantIndex);
                chunk.constants[constantIndex].print(std::cout); 
                std::printf("'\n");
                offset += 2;
                break;
            }
            case OP_ADD:      std::printf("OP_ADD\n");      offset += 1; break;
            case OP_SUBTRACT: std::printf("OP_SUBTRACT\n"); offset += 1; break;
            case OP_MULTIPLY: std::printf("OP_MULTIPLY\n"); offset += 1; break;
            case OP_DIVIDE:   std::printf("OP_DIVIDE\n");   offset += 1; break;
            case OP_RETURN:   std::printf("OP_RETURN\n");   offset += 1; break;

            case OP_DEFINE_GLOBAL:
            case OP_GET_GLOBAL: {
                const char* name = (instruction == OP_DEFINE_GLOBAL) ? "OP_DEFINE_GLOBAL" : "OP_GET_GLOBAL";
                uint8_t constantIndex = chunk.code[offset + 1];
                std::printf("%-16s %d '", name, constantIndex);
                chunk.constants[constantIndex].print(std::cout);
                std::printf("'\n");
                offset += 2;
                break;
            }

            default:
                std::printf("Unknown opcode %d\n", instruction);
                offset += 1;
                break;
        }
    }
}