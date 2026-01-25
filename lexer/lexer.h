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
	Greater,
	Smaller,
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
	Comma,
	Semicolon,
	Dot,
	False,
	True,
	End
};

struct Token {
	TokenType type;
	int value;
	std::string name;

	Token(TokenType t, double v = 0.0, std::string n = "")
		: type(t), value(v), name(n) {
	}

	Token() : type(TokenType::End), value(0.0), name("") {}
};

std::vector<Token> tokenize(const std::string& input);
std::string tokenTypeToString(TokenType type);