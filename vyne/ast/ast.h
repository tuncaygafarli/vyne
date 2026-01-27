#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>

class ASTNode;
struct Value {
    enum Type { NUMBER, STRING, ARRAY, TABLE, FUNCTION, NONE };
    Type type = NONE;

    double number = 0;
    std::string text;
    std::vector<Value> list;
    std::unordered_map<std::string, Value> table;

    struct FunctionData {
        std::vector<std::string> params;
        std::vector<std::shared_ptr<ASTNode>> body; 
    };

    std::shared_ptr<FunctionData> function;

    Value() : type(NONE) {}
    Value(double n) : type(NUMBER), number(n) {}
    Value(std::string s) : type(STRING), text(std::move(s)) {}
    Value(std::vector<Value> l) : type(ARRAY), list(std::move(l)) {}
    Value(std::unordered_map<std::string, Value> t) : type(TABLE), table(std::move(t)) {}
    Value(std::vector<std::string> p, std::vector<std::shared_ptr<ASTNode>> b) 
        : type(FUNCTION), number(0) {
        function = std::make_shared<FunctionData>();
        function->params = std::move(p);
        function->body = std::move(b);
    }

    bool operator==(const Value& other) const {
        if (type != other.type) return false;
        if (type == NUMBER) return number == other.number;
        if (type == STRING) return text == other.text;
        if (type == NONE) return true;

        return false; 
    }

    bool operator<(const Value& other) const {
        if (type != other.type) return type < other.type;

        if (type == NUMBER) return number < other.number;

        if (type == STRING) return text < other.text;

        return false;
    }

    void print(std::ostream& os) const {
        if (type == Type::NONE) return;
        if (type == Type::ARRAY) {
            os << "{";
            for (size_t i = 0; i < list.size(); ++i) {
                list[i].print(os);
                if (i < list.size() - 1) os << ", ";
            }
            os << "}";
        } if (type == Type::FUNCTION) {
            os << "<function " << (function ? "defined" : "null") << ">";
        } else if (type == Type::STRING) {
            os << "\"" << text << "\"";
        } else {
            os << number;
        }
    }

    size_t getBytes() const {
        switch(type){
            case NUMBER  :  return sizeof(double);
            case STRING  :  return text.length() * sizeof(char);
            case ARRAY   : {
                size_t total = 0; // doing base calculation here because also vectors has base size

                for(const auto& el : list){
                    total += el.getBytes();
                }

                return total;
            }

            default : return 0;
        }
    }
};

using SymbolTable  = std::unordered_map<std::string, Value>;
using SymbolContainer = std::unordered_map<std::string, SymbolTable>;

class ASTNode {
public:
    int lineNumber = 0;
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const = 0;
};

class GroupNode : public ASTNode {
    const std::string groupName;
    std::vector<std::unique_ptr<ASTNode>> statements;
public:
    GroupNode(std::string name, std::vector<std::unique_ptr<ASTNode>> stmts)
        : groupName(name), statements(std::move(stmts)) {
    }

    Value evaluate(SymbolContainer& forest, std::string currentGroup) const override;
};

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override;
};

class VariableNode : public ASTNode {
    std::string name;
    std::vector<std::string> specificGroup;
public:
    VariableNode(std::string name, std::vector<std::string>&& group = {})
        : name(std::move(name)), specificGroup(std::move(group)) {
    }

    Value evaluate(SymbolContainer& forest, std::string currentGroup) const override;

    // getters
    const std::vector<std::string>& getScope() { return specificGroup; }
    const std::string& getName()  { return name; }
};

class AssignmentNode : public ASTNode {
    std::string identifier;
    std::unique_ptr<ASTNode> rhs;
    std::vector<std::string> scopePath;
public:
    AssignmentNode(std::string id, std::unique_ptr<ASTNode> rhs_ptr, std::vector<std::string> path = {})
        : identifier(std::move(id)), rhs(std::move(rhs_ptr)), scopePath(std::move(path)) {
    }
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override;
};

class BinOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
public:
    BinOpNode(char op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {
    }
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override;
};

class PrintNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
public:
    PrintNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override;
};

class SizeofNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
public:
    SizeofNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override;
};

class StringNode : public ASTNode {
    std::string text;
public:
    StringNode(std::string t) : text(std::move(t)) {}
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override {
        return Value(text);
    }
};

class BooleanNode : public ASTNode {
    bool condition;
public :
    BooleanNode(bool c) : condition(std::move(c)) {}

    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override {
        return Value(condition);
    };
};

class ArrayNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> elements;
public:
    ArrayNode(std::vector<std::unique_ptr<ASTNode>> elm) : elements(std::move(elm)) {}
    Value evaluate(SymbolContainer& forest, std::string currentGroup = "global") const override;
};

class IndexAccessNode : public ASTNode {
    std::string name;
    std::vector<std::string> scope;
    std::unique_ptr<ASTNode> index;

public :
    IndexAccessNode(std::string n, std::vector<std::string> s, std::unique_ptr<ASTNode> idx)
        : name(std::move(n)), scope(std::move(s)), index(std::move(idx)) {}

    Value evaluate(SymbolContainer& forest, std::string currentGroup) const override;
};

class FunctionNode : public ASTNode {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::shared_ptr<ASTNode>> body;

public:
    FunctionNode(const std::string& n,std::vector<std::string> params, 
                 std::vector<std::shared_ptr<ASTNode>> body)
        : name(std::move(n)), parameters(std::move(params)), body(std::move(body)) {}

    Value evaluate(SymbolContainer& forest, std::string currentGroup) const override;
};

class FunctionCallNode : public ASTNode {
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : funcName(std::move(name)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& forest, std::string currentGroup) const override;
};

class MethodCallNode : public ASTNode {
    std::unique_ptr<ASTNode> receiver;
    std::string methodName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    MethodCallNode(std::unique_ptr<ASTNode> recv, std::string method, 
                   std::vector<std::unique_ptr<ASTNode>> args)
        : receiver(std::move(recv)), methodName(std::move(method)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& forest, std::string currentGroup) const override;
};

std::string resolvePath(std::vector<std::string> scope, std::string currentGroup);