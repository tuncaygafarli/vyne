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

double Parser::parseFactor() {
	Token current = peekToken();

	if (current.type == TokenType::Number) {
		getNextToken();
		return current.value;
	}	

	else if (current.type == TokenType::Left_Parenthese) {
		getNextToken();
		double result = parseExpression();
		consume(TokenType::Right_Parenthese);
		return result;
	}
	throw std::runtime_error("Expected a number or parenthesis");
}

double Parser::parseTerm() {
	double left = parseFactor();

	while (peekToken().type == TokenType::Multiply || peekToken().type == TokenType::Division) {
		Token op = getNextToken();
		double right = parseFactor();

		if (op.type == TokenType::Multiply) left *= right;
		else left /= right;
	}
	return left;
}

double Parser::parseExpression() {
	double left = parseTerm();

	while (peekToken().type == TokenType::Add || peekToken().type == TokenType::Substract) {
		Token op = getNextToken();
		double right = parseTerm();

		if (op.type == TokenType::Add) left += right;
		else left -= right;
	}
	return left;
}

void Parser::consume(TokenType expected) {
	Token t = getNextToken();
	if (t.type != expected) {
		throw std::runtime_error("Unexpected token!");
	}
}