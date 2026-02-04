#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <variant>
#include <sstream>
#include <functional>
#include <cstdint>
#include <cmath>

#include "../lexer/lexer.h"
#include "../types.h"
#include "value.h"

class Emitter;
class ASTNode;
struct Value; 
using SymbolTable = std::unordered_map<uint32_t, Value>;
using SymbolContainer = std::unordered_map<std::string, SymbolTable>;

// exception signals
struct ReturnException {
    Value value;
};

struct BreakException {};
struct ContinueException {};

enum class NodeType {
    PROGRAM,
    GROUP,

    NUMBER,
    STRING,
    BOOLEAN,
    VARIABLE,
    ASSIGNMENT,

    BINARY_OP,
    POSTFIX,

    ARRAY,
    RANGE,
    INDEX_ACCESS,

    FUNCTION,
    FUNCTION_CALL,
    BUILTIN_CALL,
    METHOD_CALL,
    RETURN,

    WHILE,
    FOR,
    BLOCK,
    IF,

    MODULE,
    DISMISS,

    BREAK,
    CONTINUE
};


class ASTNode {
public:
    int lineNumber = 0;
    NodeType nodeType;

    ASTNode(NodeType t) : nodeType(t) {}

    virtual ~ASTNode() = default;

    NodeType type() const { return nodeType; }
    virtual VType getStaticType() const { return VType::Unknown; }
    virtual Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const = 0;
    virtual void compile(Emitter& e) const = 0;
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;

    ProgramNode(std::vector<std::shared_ptr<ASTNode>> stmts) 
        : ASTNode(NodeType::PROGRAM), statements(std::move(stmts)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
};

class GroupNode : public ASTNode {
    const std::string groupName;
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    GroupNode(std::string name, std::vector<std::unique_ptr<ASTNode>> stmts)
        : ASTNode(NodeType::GROUP), groupName(name), statements(std::move(stmts)) {
    }

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
};

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : ASTNode(NodeType::NUMBER), value(val) {}
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Number; }
};

class VariableNode : public ASTNode {
    uint32_t nameId;
    std::string originalName; 
    std::vector<std::string> specificGroup;

public:
    VariableNode(uint32_t id, std::string name, std::vector<std::string> group = {})
        : ASTNode(NodeType::VARIABLE), nameId(id), originalName(std::move(name)), specificGroup(std::move(group)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;

    const std::vector<std::string>& getScope() const { return specificGroup; }
    uint32_t getNameId() const { return nameId; }
    const std::string& getOriginalName() const { return originalName; }
};

class AssignmentNode : public ASTNode {
    uint32_t identifierId;
    std::string originalName;
    std::unique_ptr<ASTNode> rhs;
    std::unique_ptr<ASTNode> indexExpr;
    std::vector<std::string> scopePath;
    bool isConstant;

public:
    AssignmentNode(uint32_t id, 
                   std::string on, 
                   std::unique_ptr<ASTNode> rhs_ptr, 
                   bool ic,
                   std::unique_ptr<ASTNode> idx_ptr = nullptr,
                   std::vector<std::string> path = {})
        : ASTNode(NodeType::ASSIGNMENT),
          identifierId(id), 
          originalName(std::move(on)), 
          rhs(std::move(rhs_ptr)), 
          indexExpr(std::move(idx_ptr)),
          scopePath(std::move(path)),
          isConstant(ic) {}

    void compile(Emitter& e) const override;

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class BinOpNode : public ASTNode {
    VTokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
public:
    BinOpNode(VTokenType op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : ASTNode(NodeType::BINARY_OP), op(op), left(std::move(l)), right(std::move(r)) {
    }
    
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;

    VType getStaticType() const override {
        switch(op) {
            case VTokenType::Add:
            case VTokenType::Substract:
            case VTokenType::Multiply:
            case VTokenType::Division:
            case VTokenType::Floor_Divide:
            case VTokenType::Modulo:
                return VType::Number;
            case VTokenType::And:
            case VTokenType::Or:
            case VTokenType::Double_Equals:
            case VTokenType::Greater:
            case VTokenType::Smaller:
                return VType::Number;
            default:
                return VType::Unknown;
        }
    }
};

class PostFixNode : public ASTNode {
    VTokenType op;
    std::unique_ptr<ASTNode> left;
public:
    PostFixNode(VTokenType op, std::unique_ptr<ASTNode> lhs)
        : ASTNode(NodeType::POSTFIX), op(op), left(std::move(lhs)) {}
    
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Number; }
};

class BuiltInCallNode : public ASTNode {
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> arguments;
public:
    BuiltInCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args) 
        : ASTNode(NodeType::BUILTIN_CALL), 
        funcName(std::move(name)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
};

class StringNode : public ASTNode {
    std::string text;
public:
    StringNode(std::string t) : ASTNode(NodeType::STRING), text(std::move(t)) {}
    
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override {
        return Value(text);
    }

    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::String; }
};

class BooleanNode : public ASTNode {
    bool condition;
public :
    BooleanNode(bool c) : ASTNode(NodeType::BOOLEAN), condition(c) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override {
        return Value(condition);
    };
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Number; }
};

class ArrayNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> elements;
public:
    ArrayNode(std::vector<std::unique_ptr<ASTNode>> elm) : ASTNode(NodeType::ARRAY), elements(std::move(elm)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Array; }
};

class RangeNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
public:
    RangeNode(std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r) :
    ASTNode(NodeType::RANGE),
    left(std::move(l)), right(std::move(r)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Array; }
};

class IndexAccessNode : public ASTNode {
    uint32_t nameId;
    std::string originalName;
    std::vector<std::string> scope;
    std::unique_ptr<ASTNode> index;

public :
    IndexAccessNode(uint32_t n, std::string on, std::vector<std::string> s, std::unique_ptr<ASTNode> idx)
        : ASTNode(NodeType::INDEX_ACCESS), nameId(n), originalName(std::move(on)), scope(std::move(s)), index(std::move(idx)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
};

class FunctionNode : public ASTNode {
    std::string targetModule;
    uint32_t funcNameId;
    std::string originalName;
    std::vector<uint32_t> parameterIds;
    std::vector<std::shared_ptr<ASTNode>> body;

public:
    FunctionNode(std::string tm, uint32_t n,std::string on, std::vector<uint32_t> pid, 
                 std::vector<std::shared_ptr<ASTNode>> body)
        : ASTNode(NodeType::FUNCTION), targetModule(tm), funcNameId(n), originalName(std::move(on)), parameterIds(pid), body(std::move(body)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Function; }
};

class FunctionCallNode : public ASTNode {
    uint32_t funcNameId;
    std::string originalName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    FunctionCallNode(uint32_t fn, std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : ASTNode(NodeType::FUNCTION_CALL), funcNameId(fn), originalName(std::move(name)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
};

class ReturnNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
public:
    ReturnNode(std::unique_ptr<ASTNode> expr) : ASTNode(NodeType::RETURN), expression(std::move(expr)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
};

class MethodCallNode : public ASTNode {
    std::unique_ptr<ASTNode> receiver;
    std::string methodName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    MethodCallNode(std::unique_ptr<ASTNode> recv, std::string method, 
                   std::vector<std::unique_ptr<ASTNode>> args)
        : ASTNode(NodeType::METHOD_CALL),
        receiver(std::move(recv)), methodName(std::move(method)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
};

class WhileNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

public:
    WhileNode(std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> b)
        : ASTNode(NodeType::WHILE), condition(std::move(c)), body(std::move(b)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
};

class ForNode : public ASTNode {
    enum class ForMode { LOOP, COLLECT, FILTER, EVERY };
    std::unique_ptr<ASTNode> iterable;
    std::unique_ptr<ASTNode> body;
    std::string iteratorName;
    ForMode mode;

public:
    ForNode(std::unique_ptr<ASTNode> i, std::unique_ptr<ASTNode> b, std::string in, ForMode m)
        : ASTNode(NodeType::FOR), iterable(std::move(i)), body(std::move(b)), iteratorName(std::move(in)), mode(m) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;

    static ForMode getForMode(const std::string& modeStr){
        if (modeStr == "collect") return ForNode::ForMode::COLLECT;
        else if (modeStr == "filter") return ForNode::ForMode::FILTER;
        else if (modeStr == "every") return ForNode::ForMode::EVERY;
        else return ForNode::ForMode::LOOP;
    }
};

class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;

    BlockNode(std::vector<std::shared_ptr<ASTNode>> stmts) 
        : ASTNode(NodeType::BLOCK), statements(std::move(stmts)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
    void compile(Emitter& e) const override;
};

class ModuleNode : public ASTNode {
public:
    uint32_t moduleId;
    std::string originalName;

    ModuleNode(uint32_t mId, std::string mName) : ASTNode(NodeType::MODULE), moduleId(mId), originalName(std::move(mName)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
    VType getStaticType() const override { return VType::Module; }
};

class DismissNode : public ASTNode {
public:
    uint32_t moduleId;
    std::string originalName;

    DismissNode(uint32_t mId, std::string mName) : ASTNode(NodeType::DISMISS), moduleId(mId), originalName(std::move(mName)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;
    std::unique_ptr<ASTNode> elseBody;

    IfNode(std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> b, std::unique_ptr<ASTNode> eb = nullptr) : 
    ASTNode(NodeType::IF), condition(std::move(c)), body(std::move(b)), elseBody(std::move(eb)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
    void compile(Emitter& e) const override;
};

// exception structs
struct BreakNode : public ASTNode {
    BreakNode() : ASTNode(NodeType::BREAK) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override {
        throw BreakException();
    }
    void compile(Emitter& e) const override;
};

struct ContinueNode : public ASTNode {
    ContinueNode() : ASTNode(NodeType::CONTINUE) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override {
        throw ContinueException();
    }
    void compile(Emitter& e) const override;
};

std::string resolvePath(std::vector<std::string> scope, std::string currentGroup);