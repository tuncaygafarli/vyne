#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <variant>

class ASTNode;
struct Value {
    enum TypeIndex { NONE = 0, NUMBER = 1, STRING = 2, ARRAY = 3, TABLE = 4, FUNCTION = 5 };
    struct FunctionData {
        std::vector<std::string> params;
        std::vector<std::shared_ptr<ASTNode>> body; 
    };

    using VariantData = std::variant<
        std::monostate,
        double,
        std::string,
        std::vector<Value>,
        std::unordered_map<std::string, Value>,
        std::shared_ptr<FunctionData>
    >;

    VariantData data;

    Value() : data(std::monostate{}) {}
    Value(double n) : data(n) {}
    Value(std::string s) : data(std::move(s)) {}
    Value(std::vector<Value> l) : data(std::move(l)) {}
    Value(std::shared_ptr<FunctionData> f) : data(std::move(f)) {}
    Value(std::vector<std::string> p, std::vector<std::shared_ptr<ASTNode>> b) {
        auto func = std::make_shared<FunctionData>();
        func->params = std::move(p);
        func->body = std::move(b);
        
        data = std::move(func); 
    }

    // safe getters
    int getType() const { return data.index(); }

    double asNumber() const { return std::get<double>(data); }
    const std::string& asString() const { return std::get<std::string>(data); }
    std::vector<Value>& asList() { return std::get<std::vector<Value>>(data); }
    const std::vector<Value>& asList() const { return std::get<std::vector<Value>>(data); }
    const std::shared_ptr<FunctionData>& asFunction() const { return std::get<std::shared_ptr<FunctionData>>(data); }

    void print(std::ostream& os) const {
        switch(data.index()){
            case 0 :
                os << "null";
                break;
            case 1 :
                os << std::get<double>(data);
                break;
            case 2 :
                os << "\"" << std::get<std::string>(data) << "\"";
                break;
            case 3 : {
                const auto& list = std::get<std::vector<Value>>(data);

                os << "{";
                for (size_t i = 0; i < list.size(); ++i) {
                    list[i].print(os);
                    if (i < list.size() - 1) os << ", ";
                }
                os << "}";
                break;
            }

            case 5:
                os << "<function>";
                break;
            default:
                os << "<unknown>";
                break; 
        }
    }

    size_t getBytes() const {
        switch(data.index()){
            case 1:
                return sizeof(double);
            case 2: 
                return std::get<std::string>(data).length() * sizeof(char);
            case 3: {
                size_t total = 0;

                const auto& list = std::get<std::vector<Value>>(data);
                for (const auto& v : list) {
                    total += v.getBytes();
                }

                return total;
            }

            default :
                return 0;
        }
    }

    bool operator==(const Value& other) const {
        if (data.index() != other.data.index()) return false;

        switch (data.index()) {
            case 0: return true;
            case 1: return std::get<double>(data) == std::get<double>(other.data);
            case 2: return std::get<std::string>(data) == std::get<std::string>(other.data);
            case 3: return std::get<std::vector<Value>>(data) == std::get<std::vector<Value>>(other.data);
            default: return false; 
        }
    }

    bool operator<(const Value& other) const {
        if (data.index() != other.data.index()) {
            return data.index() < other.data.index();
        }

        switch (data.index()) {
            case 1:
                return std::get<double>(data) < std::get<double>(other.data);
            case 2:
                return std::get<std::string>(data) < std::get<std::string>(other.data);
            default:
                return false;
        }
    }
};

struct ReturnException {
    Value value;
};

using SymbolTable  = std::unordered_map<std::string, Value>;
using SymbolContainer = std::unordered_map<std::string, SymbolTable>;

class ASTNode {
public:
    int lineNumber = 0;
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const = 0;
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
    std::string name;
    std::vector<std::string> specificGroup;
public:
    VariableNode(std::string name, std::vector<std::string>&& group = {})
        : name(std::move(name)), specificGroup(std::move(group)) {
    }

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;

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
    Value evaluate(SymbolContainer& env, std::string currentGroup = "global") const override;
};

class BinOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
public:
    BinOpNode(char op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
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
    BooleanNode(bool c) : condition(std::move(c)) {}

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
    std::string name;
    std::vector<std::string> scope;
    std::unique_ptr<ASTNode> index;

public :
    IndexAccessNode(std::string n, std::vector<std::string> s, std::unique_ptr<ASTNode> idx)
        : name(std::move(n)), scope(std::move(s)), index(std::move(idx)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class FunctionNode : public ASTNode {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::shared_ptr<ASTNode>> body;

public:
    FunctionNode(const std::string& n,std::vector<std::string> params, 
                 std::vector<std::shared_ptr<ASTNode>> body)
        : name(std::move(n)), parameters(std::move(params)), body(std::move(body)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class FunctionCallNode : public ASTNode {
    std::string funcName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : funcName(std::move(name)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class ReturnNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;
public:
    ReturnNode(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

class MethodCallNode : public ASTNode {
    std::unique_ptr<ASTNode> receiver;
    std::string methodName;
    std::vector<std::unique_ptr<ASTNode>> arguments;

public:
    MethodCallNode(std::unique_ptr<ASTNode> recv, std::string method, 
                   std::vector<std::unique_ptr<ASTNode>> args)
        : receiver(std::move(recv)), methodName(std::move(method)), arguments(std::move(args)) {}

    Value evaluate(SymbolContainer& env, std::string currentGroup) const override;
};

std::string resolvePath(std::vector<std::string> scope, std::string currentGroup);