#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST.h"
#include <memory>

class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
    Token& peek(size_t ahead = 0);
    Token& advance();
    bool isAtEnd();
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    void skipNewlines();
    
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<SetNode> parseSet();
    std::unique_ptr<DoNode> parseDo();
    std::unique_ptr<IfNode> parseIf();
    std::unique_ptr<PrintNode> parsePrint();
    std::unique_ptr<ResNode> parseRes();
    std::unique_ptr<RunNode> parseRun();
    std::unique_ptr<FunctionDefNode> parseFunctionDef();
    
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseLogicalOr();
    std::unique_ptr<ASTNode> parseLogicalAnd();
    std::unique_ptr<ASTNode> parseChainedComparison();
    std::unique_ptr<ASTNode> parseEquality();
    std::unique_ptr<ASTNode> parseAdditive();
    std::unique_ptr<ASTNode> parseMultiplicative();
    
    bool isComparisonOperator();
    bool checkToken(TokenType type);
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseFunctionCall();
    
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<BlockNode> parse();
};

#endif // PARSER_H
