#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

enum class VTokenType {
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
	Greater_Or_Equal,
	Smaller_Or_Equal,
	Left_Parenthese,
	Right_Parenthese,
	Identifier,
	Equals,
	And,
	Or,
	Left_CB,
	Right_CB,
	Left_Bracket,
	Right_Bracket,
	Comma,
	Semicolon,
	Dot,
	Double_Equals,
	Extends,
	Type,
	// Built-in keywords
	BuiltIn,
	While,
	Return,
	Break,
	Continue,
	Module,
	Dismiss,
	If,
	End
};

struct Token {
    VTokenType type;
    double value;
    std::string name;
    int line;

    Token(VTokenType t, int cl, double v = 0.0, std::string n = "")
        : type(t), value(v), name(n), line(cl) {
    }

    Token() : type(VTokenType::End), value(0.0), name(""), line(0) {}
};

std::vector<Token> tokenize(const std::string& input);
std::string VTokenTypeToString(VTokenType type);
char advance();