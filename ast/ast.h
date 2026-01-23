#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <map>
#include <variant>

struct Value {
	bool isString = false;
	double number = 0.0;
	std::string text = "";

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

using SymbolTable = std::map<std::string, Value>;

class ASTNode {
public :
	virtual ~ASTNode() = default;
	virtual Value evaluate(SymbolTable& env) const = 0;
};

class NumberNode : public ASTNode {
	double value;
public :
	NumberNode(double val);
	Value evaluate(SymbolTable& env) const override;
};

class VariableNode : public ASTNode {
	std::string name;
public:
	VariableNode(std::string& name);
	Value evaluate(SymbolTable& env) const override;
};

class AssignmentNode : public ASTNode {
	std::string identifier;
	std::unique_ptr<ASTNode> rhs;	
public:
	AssignmentNode(std::string id, std::unique_ptr<ASTNode> rhs_ptr);
	Value evaluate(SymbolTable& env) const override;
};

class BinOpNode : public ASTNode {
	char op;
	std::unique_ptr<ASTNode> left;
	std::unique_ptr<ASTNode> right;
public:
	BinOpNode(char op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right);
	Value evaluate(SymbolTable& env) const override;
};

class PrintNode : public ASTNode {
	std::unique_ptr<ASTNode> expression;
public:
	PrintNode(std::unique_ptr<ASTNode> expr);
	Value evaluate(SymbolTable& env) const override;
};
	
class StringNode : public ASTNode {
	std::string text;
public:
	StringNode(std::string t) : text(std::move(t)) {}
	Value evaluate(SymbolTable& env) const override {
		Value val;
		val.isString = true;
		val.text = text;
		val.number = 0;

		return val;
	}

	std::string getString() const { return text; }
};