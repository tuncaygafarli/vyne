#include "lexer.h"

std::vector<Token> tokenize(const std::string& input) {
	std::vector<Token> tokens;

	for (int i = 0; i < input.length(); i++) {
		char character = input[i];

		if (std::isspace(character)) continue;

		if (std::isdigit(character)) {
			std::string buffer;

			while (i < input.length() && (std::isdigit(input[i]) || input[i] == '.')) {
				buffer += input[i++];
			}
			--i;
			tokens.emplace_back(TokenType::Number, std::stod(buffer));
		}

		else {
			switch (character) {
			case '+': tokens.emplace_back(TokenType::Add, 0); break;
			case '-': tokens.emplace_back(TokenType::Substract, 0); break;
			case '*': tokens.emplace_back(TokenType::Multiply, 0); break;
			case '/': tokens.emplace_back(TokenType::Division, 0); break;
			case '(': tokens.emplace_back(TokenType::Left_Parenthese, 0); break;
			case ')': tokens.emplace_back(TokenType::Right_Parenthese, 0); break;
			default:
				std::cerr << "Unexpected character: " << character << std::endl;
				break;
			}
		}
	}

	tokens.emplace_back(TokenType::End, 0);
	return tokens;
}