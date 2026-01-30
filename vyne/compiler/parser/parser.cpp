#include "parser.h"
#include "../ast/value.h"

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
    return Token(TokenType::End, 0, 0, "");
}

Token Parser::peekToken() {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    return Token(TokenType::End, 0, 0, "");
}

Token Parser::lookAhead(int distance) {
    if (pos + distance < tokens.size()) {
        return tokens[pos + distance];
    }
    return Token(TokenType::End, 0, 0, "");
}

std::unique_ptr<ASTNode> Parser::parseStringLiteral() {
    Token current = peekToken(); 
    int line = current.line;

    consume(TokenType::String);

    auto node = std::make_unique<StringNode>(current.name);
    node->lineNumber = line;

    return node;
}

std::unique_ptr<ASTNode> Parser::parseNumberLiteral(){
    Token current = peekToken(); 
    int line = current.line;

    consume(TokenType::Number);
    auto node = std::make_unique<NumberNode>(current.value);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseBooleanLiteral() {
    Token tok = peekToken();
    bool value = (tok.type == TokenType::True);
    consume(tok.type);
    
    auto node = std::make_unique<BooleanNode>(value);
    node->lineNumber = tok.line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseArrayLiteral() {
    Token tok = peekToken(); 
    int line = tok.line;

    consume(TokenType::Left_Bracket);
    
    std::vector<std::unique_ptr<ASTNode>> elements;
    
    if (peekToken().type != TokenType::Right_Bracket) {
        elements.emplace_back(parseExpression());
        
        while (peekToken().type == TokenType::Comma) {
            consume(TokenType::Comma);
            elements.emplace_back(parseExpression());
        }
    }
    
    consume(TokenType::Right_Bracket);

    auto node = std::make_unique<ArrayNode>(std::move(elements));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseGroupingExpr() {
    consume(TokenType::Left_Parenthese);
    auto node = parseExpression();
    consume(TokenType::Right_Parenthese);
    return node;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDefinition() {
    Token funcTok = consume(TokenType::Function);
    int line = funcTok.line;
    
    std::string targetModule = "";
    uint32_t funcId;
    std::string funcName;

    if (peekToken().type == TokenType::Extends) {
        consume(TokenType::Extends); 
        targetModule = consume(TokenType::Identifier).name;
        
        Token actualFuncTok = consume(TokenType::Identifier);
        funcName = actualFuncTok.name;
    } 
    else {
        Token firstTok = consume(TokenType::Identifier);
        
        if (peekToken().type == TokenType::Extends) {
            consume(TokenType::Extends);
            targetModule = firstTok.name;
            Token actualFuncTok = consume(TokenType::Identifier);
            funcName = actualFuncTok.name;
        } else {
            funcName = firstTok.name;
        }
    }
    funcId = StringPool::instance().intern(funcName);

    consume(TokenType::Left_Parenthese);
    std::vector<uint32_t> params;
    if (peekToken().type != TokenType::Right_Parenthese) {
        params.push_back(StringPool::instance().intern(consume(TokenType::Identifier).name));
        while (peekToken().type == TokenType::Comma) {
            consume(TokenType::Comma);
            params.push_back(StringPool::instance().intern(consume(TokenType::Identifier).name));
        }
    }
    consume(TokenType::Right_Parenthese);

    consume(TokenType::Left_CB);
    std::vector<std::shared_ptr<ASTNode>> body;
    while (peekToken().type != TokenType::Right_CB && peekToken().type != TokenType::End) {
        body.emplace_back(parseStatement());
    }
    consume(TokenType::Right_CB);

    auto node = std::make_unique<FunctionNode>(targetModule, funcId, funcName, std::move(params), std::move(body));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseBuiltInCall() {
    Token tok = consume(TokenType::BuiltIn);
    int line = tok.line;
    
    consume(TokenType::Left_Parenthese);
    
    std::vector<std::unique_ptr<ASTNode>> args;
    if (peekToken().type != TokenType::Right_Parenthese) {
        // Use a common pattern: Parse first, then while(comma) parse next
        do {
            if (peekToken().type == TokenType::Comma) consume(TokenType::Comma);
            args.emplace_back(parseExpression());
        } while (peekToken().type == TokenType::Comma);
    }
    
    consume(TokenType::Right_Parenthese);
    
    auto node = std::make_unique<BuiltInCallNode>(tok.name, std::move(args));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseIdentifierExpr() {
    Token tok = consume(TokenType::Identifier);
    int line = tok.line;

    if (tok.name == "return" || tok.name == "sub" || tok.name == "log") {
        throw std::runtime_error("Syntax Error: Unexpected keyword '" + tok.name + "'");
    }

    uint32_t currentId = StringPool::instance().intern(tok.name);
    std::string lastName = tok.name;
    std::vector<std::string> scope;
    std::unique_ptr<ASTNode> node;

    if (peekToken().type == TokenType::Left_Parenthese) {
        consume(TokenType::Left_Parenthese);
        std::vector<std::unique_ptr<ASTNode>> args;
        if (peekToken().type != TokenType::Right_Parenthese) {
            do {
                if (peekToken().type == TokenType::Comma) consume(TokenType::Comma);
                args.emplace_back(parseExpression());
            } while (peekToken().type == TokenType::Comma);
        }
        consume(TokenType::Right_Parenthese);
        node = std::make_unique<FunctionCallNode>(currentId, lastName, std::move(args));
    } else {
        node = std::make_unique<VariableNode>(currentId, tok.name);
    }

    while (peekToken().type == TokenType::Dot || peekToken().type == TokenType::Left_Bracket) {
        if (peekToken().type == TokenType::Dot) {
            consume(TokenType::Dot);
            Token member = consume(TokenType::Identifier);

            if (peekToken().type == TokenType::Left_Parenthese) {
                consume(TokenType::Left_Parenthese);
                std::vector<std::unique_ptr<ASTNode>> args;
                if (peekToken().type != TokenType::Right_Parenthese) {
                    do {
                        if (peekToken().type == TokenType::Comma) consume(TokenType::Comma);
                        args.emplace_back(parseExpression());
                    } while (peekToken().type == TokenType::Comma);
                }
                consume(TokenType::Right_Parenthese);
                node = std::make_unique<MethodCallNode>(std::move(node), member.name, std::move(args));
            } else {
                scope.emplace_back(lastName);
                lastName = member.name;
                uint32_t memberId = StringPool::instance().intern(member.name);
                node = std::make_unique<VariableNode>(memberId, lastName, scope);
            }
        } 
        else if (peekToken().type == TokenType::Left_Bracket) {
            consume(TokenType::Left_Bracket);
            auto indexExpr = parseExpression();
            consume(TokenType::Right_Bracket);
            
            uint32_t lastId = StringPool::instance().intern(lastName);
            node = std::make_unique<IndexAccessNode>(lastId, lastName, scope, std::move(indexExpr));
        }
    }

    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    int line = peekToken().line;
    consume(TokenType::Left_CB);
    
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (peekToken().type != TokenType::Right_CB && peekToken().type != TokenType::End) {
        statements.emplace_back(parseStatement());
    }
    
    consume(TokenType::Right_CB);
    auto node = std::make_unique<BlockNode>(std::move(statements));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    int line = peekToken().line;
    consume(TokenType::If);
    
    auto condition = parseExpression();
    
    auto body = parseStatement();
    auto node = std::make_unique<IfNode>(std::move(condition), std::move(body));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseWhileLoop() {
    int line = peekToken().line;
    consume(TokenType::While);
    
    auto condition = parseExpression();
    
    auto body = parseStatement();
    auto node = std::make_unique<WhileNode>(std::move(condition), std::move(body));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    int line = peekToken().line;
    std::vector<std::string> path;

    path.emplace_back(consume(TokenType::Identifier).name);
    while (peekToken().type == TokenType::Dot) {
        consume(TokenType::Dot);
        path.emplace_back(consume(TokenType::Identifier).name);
    }

    std::string varName = path.back();
    uint32_t varId = StringPool::instance().intern(varName);
    path.pop_back();

    consume(TokenType::Equals);
    auto rhs = parseExpression();
    consumeSemicolon();

    auto node = std::make_unique<AssignmentNode>(varId, varName, std::move(rhs), path);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseGroupDefinition() {
    int line = peekToken().line;
    consume(TokenType::Group);
    
    std::string treeName = consume(TokenType::Identifier).name;
    consume(TokenType::Left_CB);
    
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (peekToken().type != TokenType::Right_CB && peekToken().type != TokenType::End) {
        statements.emplace_back(parseStatement());
    }
    
    consume(TokenType::Right_CB);
    consumeSemicolon();
    
    auto node = std::make_unique<GroupNode>(treeName, std::move(statements));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    int line = peekToken().line;
    consume(TokenType::Return);
    
    auto expr = parseExpression();
    consumeSemicolon();
    
    auto node = std::make_unique<ReturnNode>(std::move(expr));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseLoopControl() {
    Token tok = getNextToken(); // Handles Break or Continue
    consumeSemicolon();
    
    std::unique_ptr<ASTNode> node;
    if (tok.type == TokenType::Break) {
        node = std::make_unique<BreakNode>();
    } else {
        node = std::make_unique<ContinueNode>();
    }
    
    node->lineNumber = tok.line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseModuleStatement() {
    int line = peekToken().line;
    consume(TokenType::Module);
    
    Token nameToken = consume(TokenType::Identifier);
    uint32_t mId = StringPool::instance().intern(nameToken.name);
    consumeSemicolon();
    
    auto node = std::make_unique<ModuleNode>(mId, nameToken.name);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseDismissStatement() {
    int line = peekToken().line;
    consume(TokenType::Dismiss);
    
    Token nameToken = consume(TokenType::Identifier);
    uint32_t mId = StringPool::instance().intern(nameToken.name);
    consumeSemicolon();
    
    auto node = std::make_unique<DismissNode>(mId, nameToken.name);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token current = peekToken(); 
    switch (current.type) {
        case TokenType::String:                  return parseStringLiteral();
        case TokenType::Number:                  return parseNumberLiteral();
        case TokenType::True:
        case TokenType::False:                   return parseBooleanLiteral();
        case TokenType::Identifier:              return parseIdentifierExpr();
        case TokenType::Left_Bracket:            return parseArrayLiteral();
        case TokenType::Function:                return parseFunctionDefinition();
        case TokenType::Left_Parenthese:         return parseGroupingExpr();
        case TokenType::BuiltIn:                 return parseBuiltInCall();
        default:
            throw std::runtime_error("Unexpected token in factor: " + current.name);
    }

    throw std::runtime_error("Expected number, identifier, or parenthesis");
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (peekToken().type == TokenType::Multiply || peekToken().type == TokenType::Division) {
        Token opToken = getNextToken();
        auto right = parseFactor();
        auto node = std::make_unique<BinOpNode>(opToken.type, std::move(left), std::move(right));
        node->lineNumber = opToken.line;
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseRelational() {
    auto left = parseTerm();
    while (peekToken().type == TokenType::Greater || peekToken().type == TokenType::Smaller) {
        Token opToken = getNextToken();
        auto right = parseTerm();
        left = std::make_unique<BinOpNode>(opToken.type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
    auto left = parseRelational();
    while (peekToken().type == TokenType::And) {
        Token opToken = getNextToken();
        auto right = parseRelational();
        left = std::make_unique<BinOpNode>(TokenType::Or, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
    auto left = parseLogicalOr();
    while (peekToken().type == TokenType::And) {
        Token opToken = getNextToken();
        auto right = parseLogicalOr();
        left = std::make_unique<BinOpNode>(TokenType::And, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    auto left = parseLogicalAnd();
    while (peekToken().type == TokenType::Double_Equals) {
        Token opToken = getNextToken();
        auto right = parseRelational();
        left = std::make_unique<BinOpNode>(TokenType::Double_Equals, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseEquality();
    while (peekToken().type == TokenType::Add || peekToken().type == TokenType::Substract) {
        Token opToken = getNextToken();
        auto right = parseTerm();
        auto node = std::make_unique<BinOpNode>(opToken.type, std::move(left), std::move(right));
        node->lineNumber = opToken.line;
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    Token current = peekToken();
    
    switch (current.type) {
        case TokenType::Left_CB:    return parseBlock();
        case TokenType::Return:     return parseReturnStatement();
        case TokenType::If:         return parseIfStatement();
        case TokenType::While:      return parseWhileLoop();
        case TokenType::Group:      return parseGroupDefinition();
        case TokenType::Break:      
        case TokenType::Continue:   return parseLoopControl(); 
        case TokenType::Module:     return parseModuleStatement();
        case TokenType::Dismiss:    return parseDismissStatement();
        case TokenType::Identifier: {
            int checkPos = 1;
            while(lookAhead(checkPos).type == TokenType::Dot) checkPos += 2;
            
            if(lookAhead(checkPos).type == TokenType::Equals) {
                return parseAssignment();
            }
            
            auto expr = parseExpression();
            consumeSemicolon();
            return expr;
        }
        default: {
            auto expr = parseExpression();
            consumeSemicolon(); 
            return expr;
        }
    }
}

std::unique_ptr<BlockNode> Parser::parseProgram() {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (peekToken().type != TokenType::End) {
        statements.push_back(parseStatement());
    }
    return std::make_unique<BlockNode>(std::move(statements));
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

void Parser::consumeSemicolon() {
    if (peekToken().type == TokenType::Semicolon) {
        consume(TokenType::Semicolon);
    } else if (peekToken().type != TokenType::End) {
        throw std::runtime_error("Expected ';' at end of statement.");
    }
}