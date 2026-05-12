#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    // Keywords
    SET,
    IF,
    ELSE,
    RUN,
    PRINT,
    DO,
    OUT,
    RES,
    STOP,
    
    // Literals
    NUMBER,
    STRING,
    BOOLEAN,
    
    // Identifiers
    IDENTIFIER,
    
    // Operators
    ASSIGN,
    EQUALS,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LPAREN,
    RPAREN,
    COLON,
    COMMA,
    
    // Special
    NEWLINE,
    EOF_TOKEN,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;
    
    std::unordered_map<std::string, TokenType> keywords;
    
    char current() const;
    char peek(size_t ahead = 1) const;
    void advance();
    void skipWhitespace();
    void skipComment();
    Token readNumber();
    Token readString();
    Token readIdentifier();
    
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();
    void printTokens(const std::vector<Token>& tokens);
};

#endif // LEXER_H
