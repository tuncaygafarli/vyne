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

#include "../lexer/lexer.h"
#include "value.h"


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

class ASTNode {
public:
    int lineNumber = 0;
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const = 0;
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;

    ProgramNode(std::vector<std::shared_ptr<ASTNode>> stmts) 
        : statements(std::move(stmts)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class GroupNode : public ASTNode {
    const std::string groupName;
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    GroupNode(std::string name, std::vector<std::unique_ptr<ASTNode>> stmts)
        : groupName(name), statements(std::move(stmts)) {
    }

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class VariableNode : public ASTNode {
    uint32_t nameId;
    std::string originalName; 
    std::vector<std::string> specificGroup;

public:
    VariableNode(uint32_t id, std::string name, std::vector<std::string> group = {})
        : nameId(id), originalName(std::move(name)), specificGroup(std::move(group)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;

    const std::vector<std::string>& getScope() const { return specificGroup; }
    uint32_t getNameId() const { return nameId; }
    const std::string& getOriginalName() const { return originalName; }
};

class AssignmentNode : public ASTNode {
    uint32_t identifierId;
    std::string originalName;
    std::unique_ptr<ASTNode> rhs;
    std::unique_ptr<ASTNode> indexExpr; // The [j] part
    std::vector<std::string> scopePath;

public:
    // Added idx_ptr to the arguments list
    AssignmentNode(uint32_t id, 
                   std::string on, 
                   std::unique_ptr<ASTNode> rhs_ptr, 
                   std::unique_ptr<ASTNode> idx_ptr = nullptr,
                   std::vector<std::string> path = {})
        : identifierId(id), 
          originalName(std::move(on)), 
          rhs(std::move(rhs_ptr)), 
          indexExpr(std::move(idx_ptr)),
          scopePath(std::move(path)) {
    }

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class BinOpNode : public ASTNode {
    VTokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
public:
    BinOpNode(VTokenType op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {
    }
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};
class BuiltInCallNode : public ASTNode {
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> arguments;
public:
    BuiltInCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args) 
        : funcName(std::move(name)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class StringNode : public ASTNode {
    std::string text;
public:
    StringNode(std::string t) : text(std::move(t)) {}
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override {
        return Value(text);
    }
};

class BooleanNode : public ASTNode {
    bool condition;
public :
    BooleanNode(bool c) : condition(c) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override {
        return Value(condition);
    };
};

class ArrayNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> elements;
public:
    ArrayNode(std::vector<std::unique_ptr<ASTNode>> elm) : elements(std::move(elm)) {}
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class IndexAccessNode : public ASTNode {
    uint32_t nameId;
    std::string originalName;
    std::vector<std::string> scope;
    std::unique_ptr<ASTNode> index;

public :
    IndexAccessNode(uint32_t n, std::string on, std::vector<std::string> s, std::unique_ptr<ASTNode> idx)
        : nameId(n), originalName(std::move(on)), scope(std::move(s)), index(std::move(idx)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
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
        : targetModule(tm), funcNameId(n), originalName(std::move(on)), parameterIds(pid), body(std::move(body)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class FunctionCallNode : public ASTNode {
    uint32_t funcNameId;
    std::string originalName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    FunctionCallNode(uint32_t fn, std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : funcNameId(fn), originalName(std::move(name)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class ReturnNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
public:
    ReturnNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class MethodCallNode : public ASTNode {
    std::unique_ptr<ASTNode> receiver;
    std::string methodName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    MethodCallNode(std::unique_ptr<ASTNode> recv, std::string method, 
                   std::vector<std::unique_ptr<ASTNode>> args)
        : receiver(std::move(recv)), methodName(std::move(method)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

// while statements
class WhileNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

public:
    WhileNode(std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> b)
        : condition(std::move(c)), body(std::move(b)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class BlockNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> statements;

    BlockNode(std::vector<std::shared_ptr<ASTNode>> stmts) 
        : statements(std::move(stmts)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class ModuleNode : public ASTNode {
public:
    uint32_t moduleId;
    std::string originalName;

    ModuleNode(uint32_t mId, std::string mName) : moduleId(mId), originalName(std::move(mName)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class DismissNode : public ASTNode {
public:
    uint32_t moduleId;
    std::string originalName;

    DismissNode(uint32_t mId, std::string mName) : moduleId(mId), originalName(std::move(mName)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    IfNode(std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> b) : condition(std::move(c)), body(std::move(b)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

// exception structs
struct BreakNode : public ASTNode {
    Value evaluate(SymbolContainer& env, std::string currentGroup) const override {
        throw BreakException();
    }
};

struct ContinueNode : public ASTNode {
    Value evaluate(SymbolContainer& env, std::string currentGroup) const override {
        throw ContinueException();
    }
};

std::string resolvePath(std::vector<std::string> scope, std::string currentGroup);