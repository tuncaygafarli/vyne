#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

enum class TokenType {
	// Data types
	Number,
	String,
	Group,
	Function,
	False,
	True,
	Null,
	// Arithmetic operations
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
	Left_CB,
	Right_CB,
	Left_Bracket,
	Right_Bracket,
	Comma,
	Semicolon,
	Dot,
	Double_Equals,
	// Built-in keywords
	BuiltIn,
	While,
	Return,
	End
};

struct Token {
	TokenType type;
	double value;
	std::string name;

	Token(TokenType t, double v = 0.0, std::string n = "")
		: type(t), value(v), name(n) {
	}

	Token() : type(TokenType::End), value(0.0), name("") {}
};

std::vector<Token> tokenize(const std::string& input);
std::string tokenTypeToString(TokenType type);
char advance();