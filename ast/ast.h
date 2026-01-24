#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

struct Value {
    enum Type { NUMBER, STRING, ARRAY, TABLE, NONE };
    Type type = NONE;

    double number = 0;
    std::string text;
    std::vector<Value> list;
    std::unordered_map<std::string, Value> table;

    Value() : type(NONE) {}
    Value(double n) : type(NUMBER), number(n) {}
    Value(std::string s) : type(STRING), text(std::move(s)) {}
    Value(std::vector<Value> l) : type(ARRAY), list(std::move(l)) {}
    Value(std::unordered_map<std::string, Value> t) : type(TABLE), table(std::move(t)) {}

    void print(std::ostream& os) const {
        if (type == Type::ARRAY) {
            os << "{";
            for (size_t i = 0; i < list.size(); ++i) {
                list[i].print(os);
                if (i < list.size() - 1) os << ", ";
            }
            os << "}";
        } else if (type == Type::STRING) {
            os << "\"" << text << "\"";
        } else {
            os << number;
        }
    }
};

using SymbolTable = std::unordered_map<std::string, Value>;
using SymbolForest = std::unordered_map<std::string, SymbolTable>;

class ASTNode {
public:
    int lineNumber = 0;
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const = 0;
};

class GroupNode : public ASTNode {
    std::string groupName;
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    GroupNode(std::string&& name, std::vector<std::unique_ptr<ASTNode>> stmts)
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

class ArrayNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> elements;
public:
    ArrayNode(std::vector<std::unique_ptr<ASTNode>> elm) : elements(std::move(elm)) {}
    Value evaluate(SymbolForest& forest, std::string currentGroup = "default") const override;
};