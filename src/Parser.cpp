#include "../include/Parser.h"
#include <stdexcept>
#include <iostream>

namespace {

void assignNodeLoc(ASTNode* node, const Token& tok) {
    if (node) {
        node->setSourceLocation(tok.line, tok.column);
    }
}

std::string formatTokenLoc(const Token& tok) {
    return " at line " + std::to_string(tok.line) + ", column " + std::to_string(tok.column);
}

} // namespace

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

Token& Parser::peek(size_t ahead) {
    if (tokens.empty()) {
        static Token eofToken(TokenType::EOF_TOKEN, "", 0, 0);
        return eofToken;
    }
    if (current + ahead < tokens.size()) {
        return tokens[current + ahead];
    }
    return tokens.back();
}

Token& Parser::advance() {
    current++;
    return tokens[current - 1];
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::EOF_TOKEN;
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (peek().type == type) {
        return advance();
    }
    throw std::runtime_error(message + formatTokenLoc(peek()));
}

void Parser::skipNewlines() {
    while (match(TokenType::NEWLINE)) {}
}

void Parser::parseBlockBody(std::vector<std::unique_ptr<ASTNode>>& stmts) {
    skipNewlines();
    while (!isAtEnd()) {
        TokenType t = peek().type;
        if (t == TokenType::ELSE || t == TokenType::RPAREN) {
            break;
        }
        if (t == TokenType::NEWLINE) {
            advance();
            continue;
        }
        stmts.push_back(parseStatement());
        skipNewlines();
    }
}

std::unique_ptr<BlockNode> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    while (!isAtEnd()) {
        skipNewlines();
        if (isAtEnd()) break;
        
        try {
            statements.push_back(parseStatement());
        } catch (const std::exception& e) {
            std::cerr << "Parse error: " << e.what() << std::endl;
            break;
        }
    }
    
    return std::make_unique<BlockNode>(std::move(statements));
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    skipNewlines();
    
    if (match(TokenType::SET)) {
        return parseSet();
    }
    
    if (match(TokenType::DO)) {
        return parseDo();
    }
    
    if (match(TokenType::IF)) {
        return parseIf();
    }
    
    if (match(TokenType::WHILE)) {
        return parseWhile();
    }
    
    if (match(TokenType::FOR)) {
        return parseFor();
    }
    
    if (match(TokenType::PRINT)) {
        return parsePrint();
    }
    
    if (match(TokenType::OUT)) {
        auto n = std::make_unique<OutNode>();
        assignNodeLoc(n.get(), tokens[current - 1]);
        return n;
    }
    
    if (match(TokenType::RES)) {
        return parseRes();
    }
    
    if (match(TokenType::STOP)) {
        auto n = std::make_unique<StopNode>();
        assignNodeLoc(n.get(), tokens[current - 1]);
        return n;
    }
    
    if (match(TokenType::RUN)) {
        return parseRun();
    }
    
    if (peek().type == TokenType::IDENTIFIER && checkPeek(1, TokenType::LPAREN)) {
        return parseFunctionDef();
    }
    
    throw std::runtime_error("Unexpected token: " + peek().value + formatTokenLoc(peek()));
}

std::unique_ptr<SetNode> Parser::parseSet() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'set'");
    std::unique_ptr<ASTNode> value = parseExpression();
    auto n = std::make_unique<SetNode>(name.value, std::move(value));
    assignNodeLoc(n.get(), name);
    return n;
}

std::unique_ptr<DoNode> Parser::parseDo() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'do'");
    std::unique_ptr<ASTNode> value = parseExpression();
    auto n = std::make_unique<DoNode>(name.value, std::move(value));
    assignNodeLoc(n.get(), name);
    return n;
}

std::unique_ptr<IfNode> Parser::parseIf() {
    Token anchor = tokens[current - 1];
    std::unique_ptr<ASTNode> condition = parseExpression();
    consume(TokenType::COLON, "Expected ':' after if condition");
    
    std::vector<std::unique_ptr<ASTNode>> thenBranch;
    std::vector<std::unique_ptr<ASTNode>> elseBranch;
    
    parseBlockBody(thenBranch);
    
    // Check for else – only a single statement in the else branch
    if (match(TokenType::ELSE)) {
        consume(TokenType::COLON, "Expected ':' after else");
        skipNewlines();
        
        // Parse exactly one statement for the else branch
        if (!isAtEnd()) {
            elseBranch.push_back(parseStatement());
        }
    }
    
    auto n = std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    assignNodeLoc(n.get(), anchor);
    return n;
}

std::unique_ptr<WhileNode> Parser::parseWhile() {
    Token anchor = tokens[current - 1];
    std::unique_ptr<ASTNode> condition = parseExpression();
    consume(TokenType::COLON, "Expected ':' after while condition");
    std::vector<std::unique_ptr<ASTNode>> body;
    parseBlockBody(body);
    auto n = std::make_unique<WhileNode>(std::move(condition), std::move(body));
    assignNodeLoc(n.get(), anchor);
    return n;
}

std::unique_ptr<ForNode> Parser::parseFor() {
    Token anchor = tokens[current - 1];
    Token varTok = consume(TokenType::IDENTIFIER, "Expected loop variable after 'for'");
    std::unique_ptr<ASTNode> startExpr = parseExpression();
    std::unique_ptr<ASTNode> endExpr = parseExpression();
    consume(TokenType::COLON, "Expected ':' after for bounds");
    std::vector<std::unique_ptr<ASTNode>> body;
    parseBlockBody(body);
    auto n = std::make_unique<ForNode>(varTok.value, std::move(startExpr), std::move(endExpr), std::move(body));
    assignNodeLoc(n.get(), anchor);
    return n;
}

std::unique_ptr<PrintNode> Parser::parsePrint() {
    Token anchor = tokens[current - 1];
    std::unique_ptr<ASTNode> value = parseExpression();
    auto n = std::make_unique<PrintNode>(std::move(value));
    assignNodeLoc(n.get(), anchor);
    return n;
}

std::unique_ptr<ResNode> Parser::parseRes() {
    Token anchor = tokens[current - 1];
    std::unique_ptr<ASTNode> value = parseExpression();
    auto n = std::make_unique<ResNode>(std::move(value));
    assignNodeLoc(n.get(), anchor);
    return n;
}

std::unique_ptr<RunNode> Parser::parseRun() {
    Token anchor = tokens[current - 1];
    skipNewlines();
    std::unique_ptr<ASTNode> argument = parseExpression();
    auto n = std::make_unique<RunNode>(std::move(argument));
    assignNodeLoc(n.get(), anchor);
    return n;
}

std::unique_ptr<FunctionDefNode> Parser::parseFunctionDef() {
    Token name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LPAREN, "Expected '(' after function name");
    
    std::vector<std::unique_ptr<ASTNode>> body;
    
    skipNewlines();
    
    while (!isAtEnd()) {
        if (peek().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (peek().type == TokenType::RPAREN) {
            break;
        }
        if (peek().type == TokenType::IDENTIFIER && checkPeek(1, TokenType::LPAREN)) {
            break;
        }
        body.push_back(parseStatement());
        skipNewlines();
    }
    
    consume(TokenType::RPAREN, "Expected ')' after function body");
    
    auto fn = std::make_unique<FunctionDefNode>(name.value, std::move(body));
    assignNodeLoc(fn.get(), name);
    return fn;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    // Inline if expression: if condition: thenExpr [else: elseExpr]
    if (match(TokenType::IF)) {
        Token ifAnchor = tokens[current - 1];
        auto condition = parseLogicalOr();
        consume(TokenType::COLON, "Expected ':' after if condition");
        
        auto thenExpr = parseExpression();
        std::unique_ptr<ASTNode> elseExpr = nullptr;
        
        if (match(TokenType::ELSE)) {
            consume(TokenType::COLON, "Expected ':' after else");
            elseExpr = parseExpression();
        }
        
        // Create single-statement branches for clean ternary-style parsing
        std::vector<std::unique_ptr<ASTNode>> thenBranch;
        std::vector<std::unique_ptr<ASTNode>> elseBranch;
        
        thenBranch.push_back(std::move(thenExpr));
        if (elseExpr) {
            elseBranch.push_back(std::move(elseExpr));
        }
        
        auto n = std::make_unique<IfNode>(std::move(condition), 
                                    std::move(thenBranch), 
                                    std::move(elseBranch));
        assignNodeLoc(n.get(), ifAnchor);
        return n;
    }
    
    return parseLogicalOr();
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    return parseChainedComparison();
}

std::unique_ptr<ASTNode> Parser::parseChainedComparison() {
    std::vector<std::unique_ptr<ASTNode>> operands;
    std::vector<std::string> ops;
    std::vector<Token> opTokens;

    operands.push_back(parseAdditive());

    while (isComparisonOperator()) {
        Token opTok = tokens[current];
        advance();
        ops.push_back(opTok.value);
        opTokens.push_back(opTok);
        operands.push_back(parseAdditive());
    }

    if (ops.empty()) {
        return std::move(operands[0]);
    }

    // Leftmost operand
    std::unique_ptr<ASTNode> left = std::move(operands[0]);
    std::unique_ptr<ASTNode> combined = nullptr;

    for (size_t i = 0; i < ops.size(); ++i) {
        // Clone the middle operand (if there is a next comparison) before moving it
        std::unique_ptr<ASTNode> middleClone = (i < ops.size() - 1) ? operands[i+1]->clone() : nullptr;
        if (i < ops.size() - 1 && !middleClone) {
            throw std::runtime_error("Failed to clone AST node");
        }

        // Move the actual right operand into the comparison node
        std::unique_ptr<ASTNode> right = std::move(operands[i+1]);
        auto comparison = std::make_unique<BinaryOpNode>(ops[i], std::move(left), std::move(right));
        assignNodeLoc(comparison.get(), opTokens[i]);

        if (i == 0) {
            combined = std::move(comparison);
        } else {
            auto andNode = std::make_unique<BinaryOpNode>("&&", std::move(combined), std::move(comparison));
            assignNodeLoc(andNode.get(), opTokens[i]);
            combined = std::move(andNode);
        }

        // Next left operand is the cloned middle value (the same as the right we just used)
        if (middleClone) {
            left = std::move(middleClone);
        }
    }

    return combined;
}

bool Parser::isComparisonOperator() {
    return checkToken(TokenType::GREATER_THAN) ||
           checkToken(TokenType::GREATER_EQUAL) ||
           checkToken(TokenType::LESS_THAN) ||
           checkToken(TokenType::LESS_EQUAL) ||
           checkToken(TokenType::EQUALS) ||
           checkToken(TokenType::NOT_EQUAL);
}

bool Parser::checkToken(TokenType type) {
    if (current >= tokens.size()) return false;
    return tokens[current].type == type;
}

bool Parser::checkPeek(size_t ahead, TokenType type) {
    if (current + ahead >= tokens.size()) return false;
    return tokens[current + ahead].type == type;
}

std::unique_ptr<ASTNode> Parser::parseAdditive() {
    std::unique_ptr<ASTNode> expr = parseMultiplicative();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token opTok = tokens[current - 1];
        std::string op = opTok.value;
        std::unique_ptr<ASTNode> right = parseMultiplicative();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
        assignNodeLoc(expr.get(), opTok);
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseMultiplicative() {
    std::unique_ptr<ASTNode> expr = parsePrimary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        Token opTok = tokens[current - 1];
        std::string op = opTok.value;
        std::unique_ptr<ASTNode> right = parsePrimary();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
        assignNodeLoc(expr.get(), opTok);
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseLogicalOr() {
    std::unique_ptr<ASTNode> expr = parseLogicalAnd();
    
    while (match(TokenType::OR)) {
        Token opTok = tokens[current - 1];
        std::string op = opTok.value;
        std::unique_ptr<ASTNode> right = parseLogicalAnd();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
        assignNodeLoc(expr.get(), opTok);
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseLogicalAnd() {
    std::unique_ptr<ASTNode> expr = parseEquality();
    
    while (match(TokenType::AND)) {
        Token opTok = tokens[current - 1];
        std::string op = opTok.value;
        std::unique_ptr<ASTNode> right = parseEquality();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
        assignNodeLoc(expr.get(), opTok);
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    skipNewlines();
    
    if (match(TokenType::NOT)) {
        Token notTok = tokens[current - 1];
        std::unique_ptr<ASTNode> operand = parsePrimary();
        auto n = std::make_unique<UnaryOpNode>("!", std::move(operand));
        assignNodeLoc(n.get(), notTok);
        return n;
    }
    
    if (match(TokenType::NUMBER)) {
        Token tok = tokens[current - 1];
        auto n = std::make_unique<NumberNode>(std::stod(tok.value));
        assignNodeLoc(n.get(), tok);
        return n;
    }
    
    if (match(TokenType::STRING)) {
        Token tok = tokens[current - 1];
        auto n = std::make_unique<StringNode>(tok.value);
        assignNodeLoc(n.get(), tok);
        return n;
    }
    
    if (match(TokenType::BOOLEAN)) {
        Token tok = tokens[current - 1];
        bool value = (tok.value == "true");
        auto n = std::make_unique<BooleanNode>(value);
        assignNodeLoc(n.get(), tok);
        return n;
    }
    
    if (match(TokenType::IDENTIFIER)) {
        Token nameTok = tokens[current - 1];
        std::string name = nameTok.value;
        
        if (match(TokenType::LPAREN)) {
            consume(TokenType::RPAREN, "Expected ')' after function call");
            auto n = std::make_unique<FunctionCallNode>(name, std::vector<std::unique_ptr<ASTNode>>());
            assignNodeLoc(n.get(), nameTok);
            return n;
        }
        
        auto n = std::make_unique<IdentifierNode>(name);
        assignNodeLoc(n.get(), nameTok);
        return n;
    }
    
    if (match(TokenType::LPAREN)) {
        std::unique_ptr<ASTNode> expr = parseLogicalOr();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    throw std::runtime_error("Unexpected token in expression: " + peek().value + formatTokenLoc(peek()));
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall() {
    if (match(TokenType::IDENTIFIER)) {
        Token nameTok = tokens[current - 1];
        std::string name = nameTok.value;
        consume(TokenType::LPAREN, "Expected '(' after function name");
        
        std::vector<std::unique_ptr<ASTNode>> arguments;
        skipNewlines();
        
        if (!match(TokenType::RPAREN)) {
            do {
                skipNewlines();
                arguments.push_back(parseExpression());
                skipNewlines();
            } while (match(TokenType::COMMA));
            consume(TokenType::RPAREN, "Expected ')' after function arguments");
        }
        
        auto n = std::make_unique<FunctionCallNode>(name, std::move(arguments));
        assignNodeLoc(n.get(), nameTok);
        return n;
    }
    throw std::runtime_error("Expected function name" + formatTokenLoc(peek()));
}