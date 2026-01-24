#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <memory>
#include <map>

#include "../lexer/lexer.h"
#include "../ast/ast.h"

class Parser {
private:
	std::vector<Token> tokens;
	size_t pos = 0;
public:
	Parser(std::vector<Token> t) : tokens(t) {};

	Token getNextToken();
	Token peekToken();
	Token lookAhead(int distance);
	std::unique_ptr<ASTNode> parseFactor();
	std::unique_ptr<ASTNode> parseTerm();
	std::unique_ptr<ASTNode> parseRelational();
	std::unique_ptr<ASTNode> parseExpression();
	std::unique_ptr<ASTNode> parseStatement();
	Token consume(TokenType expected);
};