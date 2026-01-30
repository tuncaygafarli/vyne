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
    return Token(VTokenType::End, 0, 0, "");
}

Token Parser::peekToken() {
    if (pos < tokens.size()) {
        return tokens[pos];
    }
    return Token(VTokenType::End, 0, 0, "");
}

Token Parser::lookAhead(int distance) {
    if (pos + distance < tokens.size()) {
        return tokens[pos + distance];
    }
    return Token(VTokenType::End, 0, 0, "");
}

std::unique_ptr<ASTNode> Parser::parseStringLiteral() {
    Token current = peekToken(); 
    int line = current.line;

    consume(VTokenType::String);

    auto node = std::make_unique<StringNode>(current.name);
    node->lineNumber = line;

    return node;
}

std::unique_ptr<ASTNode> Parser::parseNumberLiteral(){
    Token current = peekToken(); 
    int line = current.line;

    consume(VTokenType::Number);
    auto node = std::make_unique<NumberNode>(current.value);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseBooleanLiteral() {
    Token tok = peekToken();
    bool value = (tok.type == VTokenType::True);
    consume(tok.type);
    
    auto node = std::make_unique<BooleanNode>(value);
    node->lineNumber = tok.line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseArrayLiteral() {
    Token tok = peekToken(); 
    int line = tok.line;

    consume(VTokenType::Left_Bracket);
    
    std::vector<std::unique_ptr<ASTNode>> elements;
    
    if (peekToken().type != VTokenType::Right_Bracket) {
        elements.emplace_back(parseExpression());
        
        while (peekToken().type == VTokenType::Comma) {
            consume(VTokenType::Comma);
            elements.emplace_back(parseExpression());
        }
    }
    
    consume(VTokenType::Right_Bracket);

    auto node = std::make_unique<ArrayNode>(std::move(elements));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseGroupingExpr() {
    consume(VTokenType::Left_Parenthese);
    auto node = parseExpression();
    consume(VTokenType::Right_Parenthese);
    return node;
}

std::unique_ptr<ASTNode> Parser::parseFunctionDefinition() {
    Token funcTok = consume(VTokenType::Function);
    int line = funcTok.line;
    
    std::string targetModule = "";
    uint32_t funcId;
    std::string funcName;

    if (peekToken().type == VTokenType::Extends) {
        consume(VTokenType::Extends); 
        targetModule = consume(VTokenType::Identifier).name;
        
        Token actualFuncTok = consume(VTokenType::Identifier);
        funcName = actualFuncTok.name;
    } 
    else {
        Token firstTok = consume(VTokenType::Identifier);
        
        if (peekToken().type == VTokenType::Extends) {
            consume(VTokenType::Extends);
            targetModule = firstTok.name;

            Token actualFuncTok = consume(VTokenType::Identifier);
            funcName = actualFuncTok.name;

        } else {
            funcName = firstTok.name;
        }
    }
       
    if(targetModule == "vcore" || targetModule == "vglib"){
        throw std::runtime_error("Permission Error : Cannot inject function '" + funcName + "' to built-in module " + targetModule + " at line " + std::to_string(line));
    }

    funcId = StringPool::instance().intern(funcName);

    consume(VTokenType::Left_Parenthese);
    std::vector<uint32_t> params;
    if (peekToken().type != VTokenType::Right_Parenthese) {
        params.push_back(StringPool::instance().intern(consume(VTokenType::Identifier).name));
        while (peekToken().type == VTokenType::Comma) {
            consume(VTokenType::Comma);
            params.push_back(StringPool::instance().intern(consume(VTokenType::Identifier).name));
        }
    }
    consume(VTokenType::Right_Parenthese);

    consume(VTokenType::Left_CB);
    std::vector<std::shared_ptr<ASTNode>> body;
    while (peekToken().type != VTokenType::Right_CB && peekToken().type != VTokenType::End) {
        body.emplace_back(parseStatement());
    }
    consume(VTokenType::Right_CB);

    auto node = std::make_unique<FunctionNode>(targetModule, funcId, funcName, std::move(params), std::move(body));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseBuiltInCall() {
    Token tok = consume(VTokenType::BuiltIn);
    int line = tok.line;
    
    consume(VTokenType::Left_Parenthese);
    
    std::vector<std::unique_ptr<ASTNode>> args;
    if (peekToken().type != VTokenType::Right_Parenthese) {
        // Use a common pattern: Parse first, then while(comma) parse next
        do {
            if (peekToken().type == VTokenType::Comma) consume(VTokenType::Comma);
            args.emplace_back(parseExpression());
        } while (peekToken().type == VTokenType::Comma);
    }
    
    consume(VTokenType::Right_Parenthese);
    
    auto node = std::make_unique<BuiltInCallNode>(tok.name, std::move(args));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseIdentifierExpr() {
    Token tok = consume(VTokenType::Identifier);
    int line = tok.line;

    if (tok.name == "return" || tok.name == "sub" || tok.name == "log") {
        throw std::runtime_error("Syntax Error: Unexpected keyword '" + tok.name + "'");
    }

    uint32_t currentId = StringPool::instance().intern(tok.name);
    std::string lastName = tok.name;
    std::vector<std::string> scope;
    std::unique_ptr<ASTNode> node;

    if (peekToken().type == VTokenType::Left_Parenthese) {
        consume(VTokenType::Left_Parenthese);
        std::vector<std::unique_ptr<ASTNode>> args;
        if (peekToken().type != VTokenType::Right_Parenthese) {
            do {
                if (peekToken().type == VTokenType::Comma) consume(VTokenType::Comma);
                args.emplace_back(parseExpression());
            } while (peekToken().type == VTokenType::Comma);
        }
        consume(VTokenType::Right_Parenthese);
        node = std::make_unique<FunctionCallNode>(currentId, lastName, std::move(args));
    } else {
        node = std::make_unique<VariableNode>(currentId, tok.name);
    }

    while (peekToken().type == VTokenType::Dot || peekToken().type == VTokenType::Left_Bracket) {
        if (peekToken().type == VTokenType::Dot) {
            consume(VTokenType::Dot);
            Token member = consume(VTokenType::Identifier);

            if (peekToken().type == VTokenType::Left_Parenthese) {
                consume(VTokenType::Left_Parenthese);
                std::vector<std::unique_ptr<ASTNode>> args;
                if (peekToken().type != VTokenType::Right_Parenthese) {
                    do {
                        if (peekToken().type == VTokenType::Comma) consume(VTokenType::Comma);
                        args.emplace_back(parseExpression());
                    } while (peekToken().type == VTokenType::Comma);
                }
                consume(VTokenType::Right_Parenthese);
                node = std::make_unique<MethodCallNode>(std::move(node), member.name, std::move(args));
            } else {
                scope.emplace_back(lastName);
                lastName = member.name;
                uint32_t memberId = StringPool::instance().intern(member.name);
                node = std::make_unique<VariableNode>(memberId, lastName, scope);
            }
        } 
        else if (peekToken().type == VTokenType::Left_Bracket) {
            consume(VTokenType::Left_Bracket);
            auto indexExpr = parseExpression();
            consume(VTokenType::Right_Bracket);
            
            uint32_t lastId = StringPool::instance().intern(lastName);
            node = std::make_unique<IndexAccessNode>(lastId, lastName, scope, std::move(indexExpr));
        }
    }

    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
    int line = peekToken().line;
    consume(VTokenType::Left_CB);
    
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (peekToken().type != VTokenType::Right_CB && peekToken().type != VTokenType::End) {
        statements.emplace_back(parseStatement());
    }
    
    consume(VTokenType::Right_CB);
    auto node = std::make_unique<BlockNode>(std::move(statements));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    int line = peekToken().line;
    consume(VTokenType::If);
    
    auto condition = parseExpression();
    
    auto body = parseStatement();
    auto node = std::make_unique<IfNode>(std::move(condition), std::move(body));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseWhileLoop() {
    int line = peekToken().line;
    consume(VTokenType::While);
    
    auto condition = parseExpression();
    
    auto body = parseStatement();
    auto node = std::make_unique<WhileNode>(std::move(condition), std::move(body));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    int line = peekToken().line;
    
    Token varTok = consume(VTokenType::Identifier);
    uint32_t varId = StringPool::instance().intern(varTok.name);
    
    std::unique_ptr<ASTNode> indexExpr = nullptr;

    if (peekToken().type == VTokenType::Left_Bracket) {
        consume(VTokenType::Left_Bracket);
        indexExpr = parseExpression();
        consume(VTokenType::Right_Bracket);
    }

    consume(VTokenType::Equals);
    auto rhs = parseExpression();
    consumeSemicolon();

    auto node = std::make_unique<AssignmentNode>(varId, varTok.name, std::move(rhs), std::move(indexExpr));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseGroupDefinition() {
    int line = peekToken().line;
    consume(VTokenType::Group);
    
    std::string treeName = consume(VTokenType::Identifier).name;
    consume(VTokenType::Left_CB);
    
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (peekToken().type != VTokenType::Right_CB && peekToken().type != VTokenType::End) {
        statements.emplace_back(parseStatement());
    }
    
    consume(VTokenType::Right_CB);
    consumeSemicolon();
    
    auto node = std::make_unique<GroupNode>(treeName, std::move(statements));
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseReturnStatement() {
    int line = peekToken().line;
    consume(VTokenType::Return);
    
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
    if (tok.type == VTokenType::Break) {
        node = std::make_unique<BreakNode>();
    } else {
        node = std::make_unique<ContinueNode>();
    }
    
    node->lineNumber = tok.line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseModuleStatement() {
    int line = peekToken().line;
    consume(VTokenType::Module);
    
    Token nameToken = consume(VTokenType::Identifier);
    uint32_t mId = StringPool::instance().intern(nameToken.name);
    consumeSemicolon();
    
    auto node = std::make_unique<ModuleNode>(mId, nameToken.name);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseDismissStatement() {
    int line = peekToken().line;
    consume(VTokenType::Dismiss);
    
    Token nameToken = consume(VTokenType::Identifier);
    uint32_t mId = StringPool::instance().intern(nameToken.name);
    consumeSemicolon();
    
    auto node = std::make_unique<DismissNode>(mId, nameToken.name);
    node->lineNumber = line;
    return node;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    Token current = peekToken(); 
    switch (current.type) {
        case VTokenType::String:                  return parseStringLiteral();
        case VTokenType::Number:                  return parseNumberLiteral();
        case VTokenType::True:
        case VTokenType::False:                   return parseBooleanLiteral();
        case VTokenType::Identifier:              return parseIdentifierExpr();
        case VTokenType::Left_Bracket:            return parseArrayLiteral();
        case VTokenType::Left_Parenthese:         return parseGroupingExpr();
        case VTokenType::BuiltIn:                 return parseBuiltInCall();
        default:
            throw std::runtime_error("Unexpected token in factor: " + current.name);
    }

    throw std::runtime_error("Expected number, identifier, or parenthesis");
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (peekToken().type == VTokenType::Multiply || peekToken().type == VTokenType::Division) {
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
    while (peekToken().type == VTokenType::Greater || peekToken().type == VTokenType::Smaller || peekToken().type == VTokenType::Greater_Or_Equal || peekToken().type == VTokenType::Smaller_Or_Equal) {
        Token opToken = getNextToken();
        auto right = parseTerm();
        left = std::make_unique<BinOpNode>(opToken.type, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
    auto left = parseRelational();
    while (peekToken().type == VTokenType::And) {
        Token opToken = getNextToken();
        auto right = parseRelational();
        left = std::make_unique<BinOpNode>(VTokenType::Or, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
    auto left = parseLogicalOr();
    while (peekToken().type == VTokenType::And) {
        Token opToken = getNextToken();
        auto right = parseLogicalOr();
        left = std::make_unique<BinOpNode>(VTokenType::And, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    auto left = parseLogicalAnd();
    while (peekToken().type == VTokenType::Double_Equals) {
        Token opToken = getNextToken();
        auto right = parseRelational();
        left = std::make_unique<BinOpNode>(VTokenType::Double_Equals, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseEquality();
    while (peekToken().type == VTokenType::Add || peekToken().type == VTokenType::Substract) {
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
        case VTokenType::Function:   return parseFunctionDefinition();
        case VTokenType::Left_CB:    return parseBlock();
        case VTokenType::Return:     return parseReturnStatement();
        case VTokenType::If:         return parseIfStatement();
        case VTokenType::While:      return parseWhileLoop();
        case VTokenType::Group:      return parseGroupDefinition();
        case VTokenType::Break:      
        case VTokenType::Continue:   return parseLoopControl(); 
        case VTokenType::Module:     return parseModuleStatement();
        case VTokenType::Dismiss:    return parseDismissStatement();
        case VTokenType::Identifier: {
            int checkPos = 1;
            while(lookAhead(checkPos).type == VTokenType::Dot || 
                lookAhead(checkPos).type == VTokenType::Left_Bracket) {
                
                if (lookAhead(checkPos).type == VTokenType::Left_Bracket) {
                    int bracketDepth = 1;
                    checkPos++;
                    while (bracketDepth > 0 && lookAhead(checkPos).type != VTokenType::End) {
                        if (lookAhead(checkPos).type == VTokenType::Left_Bracket) bracketDepth++;
                        if (lookAhead(checkPos).type == VTokenType::Right_Bracket) bracketDepth--;
                        checkPos++;
                    }
                } else {
                    checkPos += 2;
                }
            }
            
            if(lookAhead(checkPos).type == VTokenType::Equals) {
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

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (peekToken().type != VTokenType::End) {
        statements.push_back(parseStatement());
    }
    return std::make_unique<ProgramNode>(std::move(statements));
}

Token Parser::consume(VTokenType expected) {
    Token t = peekToken();
    if (t.type == expected) {
        return tokens[pos++];
    }
    throw std::runtime_error("Error: Unexpected token type! Expected " +
        VTokenTypeToString(expected) + ", but got " +
        VTokenTypeToString(peekToken().type) + " instead.");
}

void Parser::consumeSemicolon() {
    Token t = peekToken();
    if (t.type == VTokenType::Semicolon) {
        consume(VTokenType::Semicolon);
    } else if (t.type != VTokenType::End && t.type != VTokenType::Right_CB) {
        throw std::runtime_error("Runtime/Compilation Error: Expected ';' at end of statement on line " 
            + std::to_string(t.line) + 
            ", but got '" + t.name + "' instead.");
    }
}