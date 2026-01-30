#ifndef VYNE_EMITTER_H
#define VYNE_EMITTER_H

#include "chunk.h"

class Emitter {
public:
    Chunk* currentChunk;
    int currentLine;

    Emitter(Chunk* chunk) : currentChunk(chunk), currentLine(1) {}

    void emitByte(uint8_t byte) {
        currentChunk->write(byte, currentLine);
    }

    void emitBytes(uint8_t b1, uint8_t b2) {
        emitByte(b1);
        emitByte(b2);
    }

    void emitConstant(Value val) {
        int index = currentChunk->addConstant(val);
        emitBytes(OP_CONSTANT, static_cast<uint8_t>(index));
    }

    void emitReturn() {
        emitByte(OP_RETURN);
    }
};

#endif