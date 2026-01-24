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
	Identifier,
	Equals,
	Print,
	String,
	Group,
	Left_CB,
	Right_CB,
	Left_Bracket,
	Right_Bracket,
	End
};

struct Token {
	TokenType type;
	int value;
	std::string name;
};

std::vector<Token> tokenize(const std::string& input);