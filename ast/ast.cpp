#include "ast.h"
#include <stdexcept>

NumberNode::NumberNode(double val) : value(val) {}
Value NumberNode::evaluate(SymbolTable& env) const {
    Value res;
    res.isString = false;
    res.number = value;
    res.text = "";
    return res;
}

VariableNode::VariableNode(std::string& name) : name(name) {};
Value VariableNode::evaluate(SymbolTable& env) const {
	if (env.find(name) != env.end()) {
		return env[name];
	}

	throw std::runtime_error("Undefined variable " + name);
}

AssignmentNode::AssignmentNode(std::string id, std::unique_ptr<ASTNode> rhs_ptr)
    : identifier(std::move(id)), rhs(std::move(rhs_ptr)) {
}

Value AssignmentNode::evaluate(SymbolTable& env) const {
    Value val = rhs->evaluate(env);
    env[identifier] = val;
    return val;
}

BinOpNode::BinOpNode(char op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right)
    : op(op), left(std::move(left)), right(std::move(right)) {
}

Value BinOpNode::evaluate(SymbolTable& env) const {
    Value l = left->evaluate(env);
    Value r = right->evaluate(env);

    if (r.isString || l.isString) {
        throw std::runtime_error("Type Error: Cannot perform math on strings!");
    }

    Value result;
    result.isString = false;
    switch (op) {
    case '+': result.number = l.number + r.number; break;
    case '-': result.number = l.number - r.number; break;
    case '*': result.number = l.number * r.number; break;
    case '/':
        if (r.number == 0) throw std::runtime_error("Division by zero!");
        result.number = l.number / r.number;
        break;
    default: result.number = 0;
    }
    return result;
}

PrintNode::PrintNode(std::unique_ptr<ASTNode> expr)
    : expression(std::move(expr)) {
}

Value PrintNode::evaluate(SymbolTable& env) const {
    Value val = expression->evaluate(env);
    val.print();
    return val;
}