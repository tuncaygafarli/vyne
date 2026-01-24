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
		Token firstIdent = consume(TokenType::Identifier);

		if (peekToken().type == TokenType::Left_Bracket) {
			consume(TokenType::Left_Bracket);
			Token varIdent = consume(TokenType::Identifier);
			consume(TokenType::Right_Bracket);

			return std::make_unique<VariableNode>(varIdent.name, firstIdent.name);
		}
		return std::make_unique<VariableNode>(firstIdent.name);
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

// statement parser ucun nezerde tutulub, if statementler elave olunacaq
std::unique_ptr<ASTNode> Parser::parseStatement() {
	Token current = peekToken();

	if (peekToken().type == TokenType::Print) {
		getNextToken();
		consume(TokenType::Left_Parenthese);

		auto expr = parseExpression();

		consume(TokenType::Right_Parenthese);
		return std::make_unique<PrintNode>(std::move(expr));
	}

	if (peekToken().type == TokenType::Group) {
		consume(TokenType::Group);

		std::string treeName = consume(TokenType::Identifier).name;

		consume(TokenType::Left_CB);

		std::vector<std::unique_ptr<ASTNode>> statements;

		while (peekToken().type != TokenType::Right_CB && peekToken().type != TokenType::End) {
			statements.push_back(parseStatement());
		}

		consume(TokenType::Right_CB);

		return std::make_unique<GroupNode>(treeName, std::move(statements));
	}

	if (current.type == TokenType::Identifier && lookAhead(1).type == TokenType::Equals) {
		std::string name = getNextToken().name;
		consume(TokenType::Equals);

		auto rhs = parseExpression();
		return std::make_unique<AssignmentNode>(name, std::move(rhs));
	}

	return parseExpression();
}

Token Parser::consume(TokenType expected) {
	Token t = peekToken();
	std::cout << "Consuming: " << (int)t.type << " (Expected: " << (int)expected << ")\n";
	if (t.type == expected) {
		return tokens[pos++];
	}
	throw std::runtime_error("Error: Unexpected token type! Expected " +
		std::to_string((int)expected) + " but got " +
		std::to_string((int)peekToken().type));
}