#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

enum class TokenType {
	Number,
	Multiply,
	Add,
	Substract,
	Division,
	Left_Parenthese,
	Right_Parenthese,
	End
};

struct Token {
	TokenType type;
	int value;
};

std::vector<Token> tokenize(const std::string& input);