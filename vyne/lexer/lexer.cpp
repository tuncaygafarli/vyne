#include "lexer.h"

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < input.length()) {
        char character = input[i];

        if (std::isspace(character)) {
            i++;
            continue;
        }

        if (character == '"') {
            i++;
            std::string strBuffer;

            while (i < input.length() && input[i] != '"') {
                if (input[i] == '\\' && i + 1 < input.length()) {
                    if (input[i+1] == 'n') {
                        strBuffer += '\n';
                        i += 2;
                        continue;
                    } else if (input[i+1] == 't') {
                        strBuffer += '\t';
                        i += 2;
                        continue;
                    } else if (input[i+1] == '\"') {
                        strBuffer += '\"';
                        i += 2;
                        continue;
                    }
                }
                strBuffer += input[i];
                i++;
            }

            if (i < input.length()) {
                i++;
            }
            tokens.push_back(Token(TokenType::String, 0, strBuffer));
            continue;
        }

        if (std::isdigit(character)) {
            std::string buffer;
            while (i < input.length() && (std::isdigit(input[i]) || input[i] == '.')) {
                buffer += input[i++];
            }
            tokens.emplace_back(TokenType::Number, std::stod(buffer));
            continue;
        }

        if (std::isalpha(character) || character == '_') {
            std::string buffer;
            while (i < input.length() && (std::isalnum(input[i]) || input[i] == '_')) {
                buffer += input[i++];
            }

            if (buffer == "log") tokens.emplace_back(TokenType::BuiltIn, 0, buffer);
            else if (buffer == "sizeof") tokens.emplace_back(TokenType::BuiltIn, 0, buffer);
            else if (buffer == "string") tokens.emplace_back(TokenType::BuiltIn, 0, buffer);
            else if (buffer == "type") tokens.emplace_back(TokenType::BuiltIn, 0, buffer);
            else if (buffer == "group") tokens.emplace_back(TokenType::Group, 0, "");
            else if (buffer == "true") tokens.emplace_back(TokenType::True, 1, "");
            else if (buffer == "false") tokens.emplace_back(TokenType::False, 0, "");
            else if (buffer == "null") tokens.emplace_back(TokenType::Null, 0, "");
            else if (buffer == "sub") tokens.emplace_back(TokenType::Function, 0, buffer);
            else if (buffer == "return") tokens.emplace_back(TokenType::Return, 0, buffer);
            else if (buffer == "while") tokens.emplace_back(TokenType::While, 0, buffer);
            else if (buffer == "break") tokens.emplace_back(TokenType::Break, 0, buffer);
            else if (buffer == "continue") tokens.emplace_back(TokenType::Continue, 0, buffer);
            else tokens.emplace_back(TokenType::Identifier, 0, buffer);
            continue;
        }

        // single char tokens
        switch (character) {
            case '+': tokens.emplace_back(TokenType::Add, 0, ""); break;
            case '-': tokens.emplace_back(TokenType::Substract, 0, ""); break;
            case '*': tokens.emplace_back(TokenType::Multiply, 0, ""); break;
            case '/': tokens.emplace_back(TokenType::Division, 0, ""); break;
            case '(': tokens.emplace_back(TokenType::Left_Parenthese, 0, ""); break;
            case ')': tokens.emplace_back(TokenType::Right_Parenthese, 0, ""); break;
            case '{': tokens.emplace_back(TokenType::Left_CB, 0, ""); break;
            case '}': tokens.emplace_back(TokenType::Right_CB, 0, ""); break;
            case '[': tokens.emplace_back(TokenType::Left_Bracket, 0, ""); break;
            case ']': tokens.emplace_back(TokenType::Right_Bracket, 0, ""); break;
            case ',': tokens.emplace_back(TokenType::Comma, 0, ""); break;
            case ';': tokens.emplace_back(TokenType::Semicolon, 0, ""); break;
            case '<': tokens.emplace_back(TokenType::Smaller, 0, ""); break;
            case '>': tokens.emplace_back(TokenType::Greater, 0, ""); break;
            case '.': tokens.emplace_back(TokenType::Dot, 0, ""); break;
            case '=': {
                if (i + 1 < input.length() && input[i + 1] == '=') {
                    tokens.emplace_back(TokenType::Double_Equals, 0, "==");
                    i++;
                } else {
                    tokens.emplace_back(TokenType::Equals, 0, "=");
                }
                break;
            }
            case '#': {
                while (i < input.length() && input[i] != '\n') {
                    i++;
                }
                i--; 
                break;
            }
            default:
                std::cerr << "Unexpected character: " << character << std::endl;
                break;
        }
        i++;
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
        case TokenType::String:           return "String";
		case TokenType::True:             return "'true'";
		case TokenType::False:            return "'false'";
		case TokenType::Null:             return "'null'";
        case TokenType::Group:            return "Group";
        case TokenType::Left_CB:          return "'{'";
        case TokenType::Right_CB:         return "'}'";
        case TokenType::Left_Bracket:     return "'['";
        case TokenType::Right_Bracket:    return "']'";
        case TokenType::Comma:            return "','";
		case TokenType::Semicolon:        return "';'";
		case TokenType::Dot:              return "'.'";
		case TokenType::Greater:          return "'>'";
		case TokenType::Smaller:          return "'<'";
        case TokenType::While:            return "'while'";
        case TokenType::Break:            return "'break'";
        case TokenType::Continue:         return "'continue'";
		case TokenType::BuiltIn:          return "'BuiltIn'";
        case TokenType::End:              return "'end of file'";
        default:                          return "Unknown Token";
    }
}