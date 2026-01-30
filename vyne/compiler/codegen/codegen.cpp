#include "emitter.h"
#include "../ast/ast.h"

void NumberNode::compile(Emitter& e) const {
    e.emitConstant(Value(value));
}

void StringNode::compile(Emitter& e) const {
    e.emitConstant(Value(text));
}

void BooleanNode::compile(Emitter& e) const {
    e.emitConstant(Value(condition));
}

void BinOpNode::compile(Emitter& e) const {
    left->compile(e);
    right->compile(e);

    switch (op) {
        case VTokenType::Add:   e.emitByte(OP_ADD); break;
        case VTokenType::Substract:  e.emitByte(OP_SUBTRACT); break;
        case VTokenType::Multiply:   e.emitByte(OP_MULTIPLY); break;
        case VTokenType::Division:  e.emitByte(OP_DIVIDE); break;
        default: break;
    }
}

Chunk compile(std::shared_ptr<ASTNode> root) {
    Chunk chunk;
    Emitter emitter(&chunk);

    if (root) {
        root->compile(emitter);
    }

    emitter.emitReturn(); 
    return chunk;
}

void ProgramNode::compile(Emitter& e) const {
    for (const auto& stmt : statements) {
        if (stmt) stmt->compile(e);
    }
}

void GroupNode::compile(Emitter& e) const {
    for (const auto& stmt : statements) {
        if (stmt) stmt->compile(e);
    }
}

void VariableNode::compile(Emitter& e) const {
    int nameIndex = e.currentChunk->addConstant(Value(originalName));

    e.emitBytes(OP_GET_GLOBAL, static_cast<uint8_t>(nameIndex));
}

void AssignmentNode::compile(Emitter& e) const {
    rhs->compile(e);

    int nameIndex = e.currentChunk->addConstant(Value(originalName));

    e.emitBytes(OP_DEFINE_GLOBAL, (uint8_t)nameIndex);
}

void BuiltInCallNode::compile(Emitter& e) const {}
void ArrayNode::compile(Emitter& e) const {}
void IndexAccessNode::compile(Emitter& e) const {}
void FunctionNode::compile(Emitter& e) const {}
void FunctionCallNode::compile(Emitter& e) const {}
void MethodCallNode::compile(Emitter& e) const {}
void WhileNode::compile(Emitter& e) const {}
void BlockNode::compile(Emitter& e) const {
    for (const auto& stmt : statements) {
        if (stmt) stmt->compile(e);
    }
}
void ModuleNode::compile(Emitter& e) const {}
void DismissNode::compile(Emitter& e) const {}
void IfNode::compile(Emitter& e) const {}
void BreakNode::compile(Emitter& e) const {}
void ContinueNode::compile(Emitter& e) const {}

void ReturnNode::compile(Emitter& e) const {
    if (expression) expression->compile(e);
    e.emitReturn();
}