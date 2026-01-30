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

	// TODO ADD ENTRY POINT FOR STATEMENT SO IT WILL NOT BE PUBLIC MEMBER

	// --- Literal Workers ---
	std::unique_ptr<ASTNode> parseStringLiteral();
    std::unique_ptr<ASTNode> parseNumberLiteral();
    std::unique_ptr<ASTNode> parseBooleanLiteral();
    std::unique_ptr<ASTNode> parseArrayLiteral();
    std::unique_ptr<ASTNode> parseGroupingExpr();
    std::unique_ptr<ASTNode> parseIdentifierExpr();

	// --- Statement Workers ---
	std::unique_ptr<ASTNode> parseBlock();
	std::unique_ptr<ASTNode> parseReturnStatement();
	std::unique_ptr<ASTNode> parseIfStatement();
	std::unique_ptr<ASTNode> parseWhileLoop();
	std::unique_ptr<ASTNode> parseAssignment();
	std::unique_ptr<ASTNode> parseGroupDefinition();
	std::unique_ptr<ASTNode> parseModuleStatement();
	std::unique_ptr<ASTNode> parseDismissStatement();
	std::unique_ptr<ASTNode> parseLoopControl();
	std::unique_ptr<ASTNode> parseStatement();

public:
	// --- Navigation ---
    Token peekToken();
	Token getNextToken();
    Token lookAhead(int distance);
    Token consume(VTokenType expected);
    void  consumeSemicolon();

	Parser(std::vector<Token> t) : tokens(std::move(t)) {};

	std::unique_ptr<ASTNode>     parseFunctionDefinition();
	std::unique_ptr<ASTNode>     parseBuiltInCall();
	std::unique_ptr<ASTNode>     parseFactor();
	std::unique_ptr<ASTNode>     parseTerm();
	std::unique_ptr<ASTNode>     parseRelational();
	std::unique_ptr<ASTNode>     parseLogicalOr();
	std::unique_ptr<ASTNode>     parseLogicalAnd();
	std::unique_ptr<ASTNode>     parseEquality();
	std::unique_ptr<ASTNode>     parseExpression();
	std::unique_ptr<ProgramNode> parseProgram();
};