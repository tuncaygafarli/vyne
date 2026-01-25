#include "lexer.h"

std::vector<Token> tokenize(const std::string& input) {
	std::vector<Token> tokens;

	for (int i = 0; i < input.length(); i++) {
		char character = input[i];

		if (std::isspace(character)) continue;

		if (character == '"') {
			std::string buffer;
			i++;
			while (i < input.length() && input[i] != '"') {
				buffer += input[i++];
			}
			tokens.push_back({ TokenType::String, 0, buffer });
			continue;
		}

		if (std::isdigit(character)) {
			std::string buffer;

			while (i < input.length() && (std::isdigit(input[i]) || input[i] == '.')) {
				buffer += input[i++];
			}
			--i;
			tokens.emplace_back(TokenType::Number, std::stod(buffer));
		}

		else if (std::isalpha(character) || character == '_') {
			std::string buffer;
			while ((i < input.length() && std::isalnum(input[i])) || input[i] == '_' ) {
				buffer += input[i++];
			}
			--i;

			if (buffer == "log") {
				tokens.emplace_back(TokenType::Print, 0, buffer);
			}

			else if (buffer == "group") {
				tokens.emplace_back(TokenType::Group, 0, "");
			}

			else if (buffer == "true"){
				tokens.emplace_back(TokenType::True, 1, "true");
			}

			else if (buffer == "false"){
				tokens.emplace_back(TokenType::False, 0, "false");
			}

			else {
				tokens.emplace_back(TokenType::Identifier, 0, buffer);
			}
		}

		else {
			switch (character) {
			case '+' : tokens.emplace_back(TokenType::Add, 0, ""); break;
			case '-' : tokens.emplace_back(TokenType::Substract, 0, ""); break;
			case '*' : tokens.emplace_back(TokenType::Multiply, 0, ""); break;
			case '/' : tokens.emplace_back(TokenType::Division, 0, ""); break;
			case '(' : tokens.emplace_back(TokenType::Left_Parenthese, 0, ""); break;
			case ')' : tokens.emplace_back(TokenType::Right_Parenthese, 0, ""); break;
			case '=' : tokens.emplace_back(TokenType::Equals, 0, ""); break;
			case '{' : tokens.emplace_back(TokenType::Left_CB, 0, ""); break;
			case '}' : tokens.emplace_back(TokenType::Right_CB, 0, ""); break;
			case '[' : tokens.emplace_back(TokenType::Left_Bracket, 0, ""); break;
			case ']' : tokens.emplace_back(TokenType::Right_Bracket, 0, ""); break;
			case ',' : tokens.emplace_back(TokenType::Comma, 0, ""); break;
			case ';' : tokens.emplace_back(TokenType::Semicolon, 0, ""); break;
			case '<' : tokens.emplace_back(TokenType::Smaller, 0, ""); break;
			case '>' : tokens.emplace_back(TokenType::Greater, 0, ""); break;
			case '.' : tokens.emplace_back(TokenType::Dot, 0, ""); break;
			default:
				std::cerr << "Unexpected character: " << character << std::endl;
				break;
			}
		}
	}

	tokens.emplace_back(TokenType::End, 0);
	return tokens;
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Number:           return "Number";
        case TokenType::Multiply:         return "'*'";
        case TokenType::Add:              return "'+'";
        case TokenType::Substract:        return "'-'";
        case TokenType::Division:         return "'/'";
        case TokenType::Left_Parenthese:  return "'('";
        case TokenType::Right_Parenthese: return "')'";
        case TokenType::Identifier:       return "Identifier";
        case TokenType::Equals:           return "'='";
        case TokenType::Print:            return "'log'";
        case TokenType::String:           return "String";
        case TokenType::Group:            return "Group";
        case TokenType::Left_CB:          return "'{'";
        case TokenType::Right_CB:         return "'}'";
        case TokenType::Left_Bracket:     return "'['";
        case TokenType::Right_Bracket:    return "']'";
        case TokenType::Comma:            return "','";
		case TokenType::Semicolon:        return "';'";
		case TokenType::Dot:              return "'.'";
        case TokenType::End:              return "end of file";
        default:                          return "Unknown Token";
    }
}