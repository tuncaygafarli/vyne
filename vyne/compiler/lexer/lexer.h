#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cctype>

enum class VTokenType {
    // --- LITERALS & IDENTIFIERS ---
    Identifier,         // Variable and function names
    Number,             // 42, 3.14
    String,             // String
    True, False, Null,  // Keywords as literals

    // --- KEYWORDS: STRUCTURE ---
    Group,              // Class/Structure definition
    Function,           // 'sub' or 'function' keyword
    Extends,            // Inheritance/Injections
    Module,             // Namespace declaration
    Dismiss,            // Unload/Remove module
    Arrow,              // Loop sequencer
    Const,              // Constant declaration

    // --- KEYWORDS: CONTROL FLOW ---
    If,
    Else,
    While,
    Through,
    LoopMode,
    Return,
    Break,
    Continue,

    // --- OPERATORS: ARITHMETIC ---
    Add,                // +
    Substract,          // -
    Multiply,           // *
    Division,           // /
    Double_Increment,   // ++
    Double_Decrement,   // --
    Floor_Divide,       //
    Modulo,             // %

    // --- OPERATORS: LOGIC & RELATIONAL ---
    And, Or,            // &&, ||
    Equals,             // = (Assignment)
    Double_Equals,      // == (Comparison)
    Greater,            // >
    Smaller,            // <
    Greater_Or_Equal,   // >=
    Smaller_Or_Equal,   // <=
    Pipeline,           // |>

    // --- DELIMITERS & SYMBOLS ---
    Left_Parenthese,    // (
    Right_Parenthese,   // )
    Left_CB,            // {
    Right_CB,           // }
    Left_Bracket,       // [
    Right_Bracket,      // ]
    Comma,              // ,
    Semicolon,          // ;
    Dot,                // .
    Double_Dot,         // ..

    // --- SPECIAL ---
    BuiltIn,            // Pre-defined functions
    End                 // End of File (EOF)
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