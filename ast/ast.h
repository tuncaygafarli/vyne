#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

struct Value {
    bool isString = false;
    double number = 0.0;
    std::string text = "";

    Value(std::string s) : isString(true), text(s), number(0.0) {}
    Value(double d) : isString(false), number(d), text("") {} // Added for convenience
    Value() {}

    Value& operator=(double d) {
        isString = false;
        number = d;
        return *this;
    }

    Value& operator=(const std::string& s) {
        isString = true;
        text = s;
        return *this;
    }

    void print() const {
        if (isString) std::cout << text << "\n";
        else std::cout << number << "\n";
    }
};

using SymbolTable = std::unordered_map<std::string, Value>;
using SymbolForest = std::unordered_map<std::string, SymbolTable>;

class ASTNode {
public:
    int lineNumber = 0; // Highly recommended for those errors!
    virtual ~ASTNode() = default;
    // Every subclass MUST use this exact signature
    virtual Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const = 0;
};

class GroupNode : public ASTNode {
    std::string groupName;
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    GroupNode(std::string name, std::vector<std::unique_ptr<ASTNode>> stmts)
        : groupName(name), statements(std::move(stmts)) {
    }

    Value evaluate(SymbolForest& forest, std::string currentGroup) const override;
};

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const override;
};

class VariableNode : public ASTNode {
    std::string name;
    std::string specificGroup;
public:
    VariableNode(std::string name, std::string group = "")
        : name(name), specificGroup(group) {
    }

    Value evaluate(SymbolForest& forest, std::string currentGroup) const override;
};

class AssignmentNode : public ASTNode {
    std::string identifier;
    std::unique_ptr<ASTNode> rhs;
public:
    AssignmentNode(std::string id, std::unique_ptr<ASTNode> rhs_ptr)
        : identifier(id), rhs(std::move(rhs_ptr)) {
    }
    Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const override;
};

class BinOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
public:
    BinOpNode(char op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {
    }
    Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const override;
};

class PrintNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
public:
    PrintNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const override;
};

class StringNode : public ASTNode {
    std::string text;
public:
    StringNode(std::string t) : text(std::move(t)) {}
    Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const override {
        return Value(text);
    }
};