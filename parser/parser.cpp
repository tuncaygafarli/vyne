#include "parser.h"

Token Parser::getNextToken() {
	if (pos < tokens.size()) {
		return tokens[pos++];
	}
	return { TokenType::End, 0 };
}

Token Parser::peekToken() {
	if (pos < tokens.size()) {
		return tokens[pos];
	}
	return { TokenType::End, 0 };
}

Token Parser::lookAhead(int distance) {
	if (pos + distance < tokens.size()) {
		return tokens[pos + distance];
	}
	return { TokenType::End, 0, "" };
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
	Token current = peekToken();

	if (current.type == TokenType::String) {
		getNextToken();
		return std::make_unique<StringNode>(current.name);
	}

	if (current.type == TokenType::Number) {
		getNextToken();
		return std::make_unique<NumberNode>(current.value);
	}

	if (current.type == TokenType::Identifier) {
		getNextToken();
		return std::make_unique<VariableNode>(current.name);
	}

	if (current.type == TokenType::Left_Parenthese) {
		getNextToken();
		auto node = parseExpression();
		consume(TokenType::Right_Parenthese);
		return node;
	}
	throw std::runtime_error("Expected number, identifier, or parenthesis");
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
	auto left = parseFactor();

	while (peekToken().type == TokenType::Multiply || peekToken().type == TokenType::Division) {
		Token opToken = getNextToken();
		char opChar = (opToken.type == TokenType::Multiply) ? '*' : '/';

		auto right = parseFactor();
		left = std::make_unique<BinOpNode>(opChar, std::move(left), std::move(right));
	}
	return left;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
	auto left = parseTerm();

	while (peekToken().type == TokenType::Add || peekToken().type == TokenType::Substract) {
		Token opToken = getNextToken();
		char opChar = (opToken.type == TokenType::Add) ? '+' : '-';

		auto right = parseTerm();

			left = std::make_unique<BinOpNode>(opChar, std::move(left), std::move(right));
	}
	return left;
}

// statement parser ucun nezerde tutulub, demo
std::unique_ptr<ASTNode> Parser::parseStatement() {
	Token current = peekToken();

	if (peekToken().type == TokenType::Print) {
		getNextToken();
		consume(TokenType::Left_Parenthese);

		auto expr = parseExpression();

		consume(TokenType::Right_Parenthese);
		return std::make_unique<PrintNode>(std::move(expr));
	}

	if (current.type == TokenType::Identifier && lookAhead(1).type == TokenType::Equals) {
		std::string name = getNextToken().name;
		consume(TokenType::Equals);

		auto rhs = parseExpression();
		return std::make_unique<AssignmentNode>(name, std::move(rhs));
	}

	return parseExpression();
}

void Parser::consume(TokenType expected) {
	Token t = getNextToken();
	if (t.type != expected) {
		throw std::runtime_error("Unexpected token!");
	}
}