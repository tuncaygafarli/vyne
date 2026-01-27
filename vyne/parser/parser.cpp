#include "parser.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

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

	// PARSE STRINGS
	if (current.type == TokenType::String) {
		getNextToken();
		return std::make_unique<StringNode>(current.name);
	}

	// PARSE NUMBERS
	if (current.type == TokenType::Number) {
		getNextToken();
		return std::make_unique<NumberNode>(current.value);
	}

	// PARSE BOOLEANS
	if (current.type == TokenType::True) {
		consume(TokenType::True);
		return std::make_unique<BooleanNode>(true);
	}

	if (current.type == TokenType::False) {
		consume(TokenType::False);
		return std::make_unique<BooleanNode>(false);
	}
	// PARSE IDENTIFIERS
	// x.pos = 5;
	if (current.type == TokenType::Identifier) {
		std::vector<std::string> scope;
		Token tok = consume(TokenType::Identifier);
		
		std::unique_ptr<ASTNode> node = std::make_unique<VariableNode>(tok.name);
		std::string lastName = tok.name;

		if(peekToken().type == TokenType::Left_Parenthese){
			consume(TokenType::Left_Parenthese);
			std::vector<std::unique_ptr<ASTNode>> args;

			if(peekToken().type != TokenType::Right_Parenthese){
				args.emplace_back(parseExpression());
				while (peekToken().type == TokenType::Comma) {
					consume(TokenType::Comma);
					args.emplace_back(parseExpression());
				}

				consume(TokenType::Right_Parenthese);
        
        		return std::make_unique<FunctionCallNode>(std::move(lastName), std::move(args));
			}
		}

		while (peekToken().type == TokenType::Dot || peekToken().type == TokenType::Left_Bracket) {
			
			if (peekToken().type == TokenType::Dot) {
				consume(TokenType::Dot);
				Token member = consume(TokenType::Identifier);

				// if it sees (, then it is definitely a method
				// so it will try to parse its args
				if (peekToken().type == TokenType::Left_Parenthese) {
					consume(TokenType::Left_Parenthese);
					
					std::vector<std::unique_ptr<ASTNode>> args;
					if (peekToken().type != TokenType::Right_Parenthese) {
						args.emplace_back(parseExpression());
						while (peekToken().type == TokenType::Comma) {
							consume(TokenType::Comma);
							args.emplace_back(parseExpression());
						}
					}
					consume(TokenType::Right_Parenthese);
					consume(TokenType::Semicolon);

					node = std::make_unique<MethodCallNode>(std::move(node), member.name, std::move(args));
				} 
				else {
					scope.emplace_back(lastName);
					lastName = member.name;
					node = std::make_unique<VariableNode>(lastName, std::vector<std::string>(scope));
				}
			} 
			else if (peekToken().type == TokenType::Left_Bracket) {
				consume(TokenType::Left_Bracket);
				auto indexExpr = parseExpression();
				consume(TokenType::Right_Bracket);
				consume(TokenType::Semicolon);

				node = std::make_unique<IndexAccessNode>(lastName, scope, std::move(indexExpr));
			}
		}
		return node;
	}

	if (current.type == TokenType::Left_Parenthese) {
		getNextToken();
		auto node = parseExpression();
		consume(TokenType::Right_Parenthese);
		return node;
	}

	if(current.type == TokenType::Left_CB){
		consume(TokenType::Left_CB);

		std::vector<std::unique_ptr<ASTNode>> elements;

		if(peekToken().type != TokenType::Right_CB){
			elements.emplace_back(parseExpression());

			while(peekToken().type == TokenType::Comma){
				consume(TokenType::Comma);
				elements.emplace_back(parseExpression());
			}
		}

		consume(TokenType::Right_CB);
		return std::make_unique<ArrayNode>(std::move(elements));
	}

	// parse functions
		if(current.type == TokenType::Function){
		consume(TokenType::Function);
		std::string funcName = consume(TokenType::Identifier).name;

		// parser design for params
		consume(TokenType::Left_Parenthese);
		std::vector<std::string> params;
		if(peekToken().type != TokenType::Right_Parenthese){
			params.emplace_back(consume(TokenType::Identifier).name);
			while (peekToken().type == TokenType::Comma) {
				consume(TokenType::Comma);
				params.push_back(consume(TokenType::Identifier).name);
			}
		}
		consume(TokenType::Right_Parenthese);

		// parser design for bodies
		consume(TokenType::Left_CB);
		std::vector<std::shared_ptr<ASTNode>> body;
		while(peekToken().type != TokenType::Right_CB && peekToken().type != TokenType::End){
			body.emplace_back(parseStatement());
		}
		consume(TokenType::Right_CB);

		return std::make_unique<FunctionNode>(std::move(funcName),std::move(params), std::move(body));
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

std::unique_ptr<ASTNode> Parser::parseRelational() {
	auto left = parseTerm();

	while (peekToken().type == TokenType::Greater || peekToken().type == TokenType::Smaller) {
		Token opToken = getNextToken();
		char opChar = (opToken.type == TokenType::Greater) ? '>' : '<';

		auto right = parseTerm();

		left = std::make_unique<BinOpNode>(opChar, std::move(left), std::move(right));
	}
	return left;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
	auto left = parseRelational();

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

	// assignment with scope, thanks to ferhad
	if(current.type == TokenType::Identifier){

		int checkPos = 1;

		while(lookAhead(checkPos).type == TokenType::Dot){
			checkPos += 2;
		}

		if(lookAhead(checkPos).type == TokenType::Equals){
			std::vector<std::string> path;
			path.emplace_back(consume(TokenType::Identifier).name);

			while (peekToken().type == TokenType::Dot) {
				consume(TokenType::Dot);
				path.emplace_back(consume(TokenType::Identifier).name);
			}

			std::string varName = path.back();
			path.pop_back();

			consume(TokenType::Equals);
			auto rhs = parseExpression();
			consume(TokenType::Semicolon);

			return std::make_unique<AssignmentNode>(varName, std::move(rhs), path);
		}
	}

	// log function
	if (peekToken().type == TokenType::Print) {
		getNextToken();
		consume(TokenType::Left_Parenthese);

		auto expr = parseExpression();

		consume(TokenType::Right_Parenthese);
		consume(TokenType::Semicolon);
		return std::make_unique<PrintNode>(std::move(expr));
	}

	// sizeof fnc
	if (peekToken().type == TokenType::Sizeof) {
		getNextToken();
		consume(TokenType::Left_Parenthese);

		auto expr = parseExpression();

		consume(TokenType::Right_Parenthese);
		consume(TokenType::Semicolon);
		return std::make_unique<SizeofNode>(std::move(expr));
	}

	// parsing group nodes
	if (peekToken().type == TokenType::Group) {
		consume(TokenType::Group);

		std::string treeName = consume(TokenType::Identifier).name;

		consume(TokenType::Left_CB);

		std::vector<std::unique_ptr<ASTNode>> statements;

		while (peekToken().type != TokenType::Right_CB && peekToken().type != TokenType::End) {
			statements.emplace_back(parseStatement());
		}

		consume(TokenType::Right_CB);
		consume(TokenType::Semicolon);

		return std::make_unique<GroupNode>(treeName, std::move(statements));
	}
	
	return parseExpression();
}

Token Parser::consume(TokenType expected) {
	Token t = peekToken();
	if (t.type == expected) {
		return tokens[pos++];
	}
	throw std::runtime_error("Error: Unexpected token type! Expected " +
		tokenTypeToString(expected) + ", but got " +
		tokenTypeToString(peekToken().type) + " instead.");
}
