#include "../include/Parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

Token& Parser::peek(size_t ahead) {
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
    throw std::runtime_error(message + " at line " + std::to_string(peek().line));
}

void Parser::skipNewlines() {
    while (match(TokenType::NEWLINE)) {}
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
    
    if (match(TokenType::PRINT)) {
        return parsePrint();
    }
    
    if (match(TokenType::OUT)) {
        return std::make_unique<OutNode>();
    }
    
    if (match(TokenType::RES)) {
        return parseRes();
    }
    
    if (match(TokenType::STOP)) {
        return std::make_unique<StopNode>();
    }
    
    if (match(TokenType::RUN)) {
        return parseRun();
    }
    
    if (peek().type == TokenType::IDENTIFIER && peek(1).type == TokenType::LPAREN) {
        return parseFunctionDef();
    }
    
    throw std::runtime_error("Unexpected token: " + peek().value);
}

std::unique_ptr<SetNode> Parser::parseSet() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'set'");
    std::unique_ptr<ASTNode> value = parseExpression();
    return std::make_unique<SetNode>(name.value, std::move(value));
}

std::unique_ptr<DoNode> Parser::parseDo() {
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'do'");
    std::unique_ptr<ASTNode> value = parseExpression();
    return std::make_unique<DoNode>(name.value, std::move(value));
}

std::unique_ptr<IfNode> Parser::parseIf() {
    std::unique_ptr<ASTNode> condition = parseExpression();
    consume(TokenType::COLON, "Expected ':' after if condition");
    
    std::vector<std::unique_ptr<ASTNode>> thenBranch;
    std::vector<std::unique_ptr<ASTNode>> elseBranch;
    
    skipNewlines();
    
    // Parse then branch: multiple statements
    while (!isAtEnd()) {
        // End of then branch conditions
        TokenType t = peek().type;
        if (t == TokenType::ELSE || t == TokenType::RPAREN) {
            break;
        }
        if (t == TokenType::IDENTIFIER && peek(1).type == TokenType::LPAREN) {
            break; // start of a new function definition
        }
        if (t == TokenType::NEWLINE) {
            advance();
            continue;
        }
        thenBranch.push_back(parseStatement());
        skipNewlines();
    }
    
    // Check for else – only a single statement in the else branch
    if (match(TokenType::ELSE)) {
        consume(TokenType::COLON, "Expected ':' after else");
        skipNewlines();
        
        // Parse exactly one statement for the else branch
        if (!isAtEnd()) {
            // Avoid capturing a new function definition as the else statement
            if (!(peek().type == TokenType::IDENTIFIER && peek(1).type == TokenType::LPAREN)) {
                elseBranch.push_back(parseStatement());
            }
        }
    }
    
    return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<PrintNode> Parser::parsePrint() {
    std::unique_ptr<ASTNode> value = parseExpression();
    return std::make_unique<PrintNode>(std::move(value));
}

std::unique_ptr<ResNode> Parser::parseRes() {
    std::unique_ptr<ASTNode> value = parseExpression();
    return std::make_unique<ResNode>(std::move(value));
}

std::unique_ptr<RunNode> Parser::parseRun() {
    skipNewlines();
    std::unique_ptr<ASTNode> argument = parseExpression();
    return std::make_unique<RunNode>(std::move(argument));
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
        if (peek().type == TokenType::IDENTIFIER && peek(1).type == TokenType::LPAREN) {
            break;
        }
        body.push_back(parseStatement());
        skipNewlines();
    }
    
    consume(TokenType::RPAREN, "Expected ')' after function body");
    
    return std::make_unique<FunctionDefNode>(name.value, std::move(body));
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    // Inline if expression
    if (match(TokenType::IF)) {
        std::unique_ptr<ASTNode> condition = parseEquality();
        consume(TokenType::COLON, "Expected ':' after if condition");
        
        std::vector<std::unique_ptr<ASTNode>> thenBranch;
        std::vector<std::unique_ptr<ASTNode>> elseBranch;
        
        skipNewlines();
        
        // Parse then branch (multiple statements inside inline if)
        while (!isAtEnd()) {
            if (peek().type == TokenType::RPAREN) {
                break;
            }
            // Only break on else if it's at the same indentation level (not nested)
            if (peek().type == TokenType::ELSE) {
                // Check if this else belongs to this if or a nested one
                // Simple heuristic: if we're not inside a nested if, this else belongs to us
                break;
            }
            if (peek().type == TokenType::IDENTIFIER && peek(1).type == TokenType::LPAREN) {
                break;
            }
            if (peek().type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            // Parse statements for if expressions without executing immediately
            if (match(TokenType::PRINT)) {
                std::unique_ptr<ASTNode> value = parseExpression();
                thenBranch.push_back(std::make_unique<PrintNode>(std::move(value)));
            } else if (match(TokenType::SET)) {
                Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'set'");
                std::unique_ptr<ASTNode> value = parseExpression();
                thenBranch.push_back(std::make_unique<SetNode>(name.value, std::move(value)));
            } else if (match(TokenType::DO)) {
                Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'do'");
                std::unique_ptr<ASTNode> value = parseExpression();
                thenBranch.push_back(std::make_unique<DoNode>(name.value, std::move(value)));
            } else if (match(TokenType::RUN)) {
                std::unique_ptr<ASTNode> argument = parseExpression();
                thenBranch.push_back(std::make_unique<RunNode>(std::move(argument)));
            } else if (match(TokenType::RES)) {
                std::unique_ptr<ASTNode> value = parseExpression();
                thenBranch.push_back(std::make_unique<ResNode>(std::move(value)));
            } else if (match(TokenType::OUT)) {
                thenBranch.push_back(std::make_unique<OutNode>());
            } else if (match(TokenType::STOP)) {
                thenBranch.push_back(std::make_unique<StopNode>());
            } else if (match(TokenType::IF)) {
                // Handle nested if expressions
                thenBranch.push_back(parseExpression());
            } else if (peek().type == TokenType::COLON) {
                // This is likely a nested if expression that we're already parsing
                // Skip the colon as it's part of the nested structure
                advance();
            } else if (peek().type == TokenType::ELSE) {
                // This is likely part of a nested if expression
                // Skip the else token as it's handled by the nested if parsing
                advance();
            } else {
                thenBranch.push_back(parseExpression());
            }
            skipNewlines();
        }
        
        // Check for else in inline if
        if (match(TokenType::ELSE)) {
            consume(TokenType::COLON, "Expected ':' after else");
            skipNewlines();
            
            // Parse else branch (multiple statements inside inline if)
            while (!isAtEnd()) {
                if (peek().type == TokenType::RPAREN) {
                    break;
                }
                if (peek().type == TokenType::IDENTIFIER && peek(1).type == TokenType::LPAREN) {
                    break;
                }
                if (peek().type == TokenType::IF) {
                    break; // Don't parse nested if statements in else branch
                }
                if (peek().type == TokenType::NEWLINE) {
                    advance();
                    continue;
                }
                // Parse statements for if expressions without executing immediately
                if (match(TokenType::PRINT)) {
                    std::unique_ptr<ASTNode> value = parseExpression();
                    elseBranch.push_back(std::make_unique<PrintNode>(std::move(value)));
                } else if (match(TokenType::SET)) {
                    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'set'");
                    std::unique_ptr<ASTNode> value = parseExpression();
                    elseBranch.push_back(std::make_unique<SetNode>(name.value, std::move(value)));
                } else if (match(TokenType::DO)) {
                    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'do'");
                    std::unique_ptr<ASTNode> value = parseExpression();
                    elseBranch.push_back(std::make_unique<DoNode>(name.value, std::move(value)));
                } else if (match(TokenType::RUN)) {
                    std::unique_ptr<ASTNode> argument = parseExpression();
                    elseBranch.push_back(std::make_unique<RunNode>(std::move(argument)));
                } else if (match(TokenType::RES)) {
                    std::unique_ptr<ASTNode> value = parseExpression();
                    elseBranch.push_back(std::make_unique<ResNode>(std::move(value)));
                } else if (match(TokenType::OUT)) {
                    elseBranch.push_back(std::make_unique<OutNode>());
                } else if (match(TokenType::STOP)) {
                    elseBranch.push_back(std::make_unique<StopNode>());
                } else if (match(TokenType::IF)) {
                    // Handle nested if expressions in else branch
                    elseBranch.push_back(parseExpression());
                } else if (peek().type == TokenType::COLON) {
                    // This is likely a nested if expression that we're already parsing
                    // Skip the colon as it's part of the nested structure
                    advance();
                } else if (peek().type == TokenType::ELSE) {
                    // This is likely part of a nested if expression
                    // Skip the else token as it's handled by the nested if parsing
                    advance();
                } else {
                    elseBranch.push_back(parseExpression());
                }
                skipNewlines();
            }
        }
        
        return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }
    
    return parseEquality();
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    std::unique_ptr<ASTNode> expr = parseAdditive();
    
    while (match(TokenType::EQUALS)) {
        std::string op = tokens[current - 1].value;
        std::unique_ptr<ASTNode> right = parseAdditive();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseAdditive() {
    std::unique_ptr<ASTNode> expr = parseMultiplicative();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = tokens[current - 1].value;
        std::unique_ptr<ASTNode> right = parseMultiplicative();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseMultiplicative() {
    std::unique_ptr<ASTNode> expr = parsePrimary();
    
    while (match(TokenType::MULTIPLY) || match(TokenType::DIVIDE)) {
        std::string op = tokens[current - 1].value;
        std::unique_ptr<ASTNode> right = parsePrimary();
        expr = std::make_unique<BinaryOpNode>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    skipNewlines();
    
    if (match(TokenType::NUMBER)) {
        return std::make_unique<NumberNode>(std::stod(tokens[current - 1].value));
    }
    
    if (match(TokenType::STRING)) {
        return std::make_unique<StringNode>(tokens[current - 1].value);
    }
    
    if (match(TokenType::BOOLEAN)) {
        bool value = (tokens[current - 1].value == "true");
        return std::make_unique<BooleanNode>(value);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        std::string name = tokens[current - 1].value;
        
        if (match(TokenType::LPAREN)) {
            consume(TokenType::RPAREN, "Expected ')' after function call");
            return std::make_unique<FunctionCallNode>(name, std::vector<std::unique_ptr<ASTNode>>());
        }
        
        return std::make_unique<IdentifierNode>(name);
    }
    
    if (match(TokenType::LPAREN)) {
        std::unique_ptr<ASTNode> expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    if (match(TokenType::RUN)) {
        skipNewlines();
        std::unique_ptr<ASTNode> argument = parseExpression();
        return std::make_unique<RunNode>(std::move(argument));
    }
    
    throw std::runtime_error("Unexpected token in expression: " + peek().value);
}

std::unique_ptr<ASTNode> Parser::parseFunctionCall() {
    if (match(TokenType::IDENTIFIER)) {
        std::string name = tokens[current - 1].value;
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
        
        return std::make_unique<FunctionCallNode>(name, std::move(arguments));
    }
    throw std::runtime_error("Expected function name");
}