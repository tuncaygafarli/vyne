#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

#include "../lexer/lexer.h"

class Parser {
private:
	std::vector<Token> tokens;
	size_t pos = 0;
public:
	Parser(std::vector<Token> t) : tokens(t) {};

	Token getNextToken();
	Token peekToken();
	double parseFactor();
	double parseTerm();
	double parseExpression();
	void consume(TokenType expected);
};