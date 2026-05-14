#include "../include/Lexer.h"
#include <iostream>
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& source) 
    : source(source), position(0), line(1), column(1),
      keywords({
        {"set", TokenType::SET},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"run", TokenType::RUN},
        {"print", TokenType::PRINT},
        {"do", TokenType::DO},
        {"out", TokenType::OUT},
        {"res", TokenType::RES},
        {"stop", TokenType::STOP},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"true", TokenType::BOOLEAN},
        {"false", TokenType::BOOLEAN},
        {"exec", TokenType::EXEC}
      }) {}

char Lexer::current() const {
    return position < source.length() ? source[position] : '\0';
}

char Lexer::peek(size_t ahead) const {
    return position + ahead < source.length() ? source[position + ahead] : '\0';
}

void Lexer::advance() {
    if (current() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    position++;
}

void Lexer::skipWhitespace() {
    while (isspace(current()) && current() != '\n') {
        advance();
    }
}

void Lexer::skipComment() {
    if (current() == '/' && peek() == '/') {
        while (current() != '\n' && current() != '\0') {
            advance();
        }
    }
}

Token Lexer::readNumber() {
    int startLine = line;
    int startColumn = column;
    std::string number;
    
    while (isdigit(current()) || current() == '.') {
        number += current();
        advance();
    }
    
    return Token(TokenType::NUMBER, number, startLine, startColumn);
}

Token Lexer::readString() {
    int startLine = line;
    int startColumn = column;
    advance(); // Skip opening quote
    std::string str;
    
    while (current() != '"' && current() != '\0') {
        if (current() == '\\') {
            advance();
            if (current() != '\0') {
                // Convert escape sequences to actual characters
                switch (current()) {
                    case 'n':
                        str += '\n';
                        break;
                    case 't':
                        str += '\t';
                        break;
                    case 'r':
                        str += '\r';
                        break;
                    case '\\':
                        str += '\\';
                        break;
                    case '"':
                        str += '"';
                        break;
                    case '\'':
                        str += '\'';
                        break;
                    default:
                        // Unknown escape sequence, keep the character
                        str += current();
                        break;
                }
                advance();
            }
        } else {
            str += current();
            advance();
        }
    }
    
    if (current() == '"') {
        advance(); // Skip closing quote
    }
    
    return Token(TokenType::STRING, str, startLine, startColumn);
}

Token Lexer::readIdentifier() {
    int startLine = line;
    int startColumn = column;
    std::string identifier;
    
    while (isalnum(current()) || current() == '_') {
        identifier += current();
        advance();
    }
    
    // Check if it's a keyword
    auto it = keywords.find(identifier);
    TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    
    return Token(type, identifier, startLine, startColumn);
}

Token Lexer::readFlag() {
    int startLine = line;
    int startColumn = column;
    std::string flag;
    
    // Skip the first dash
    advance();
    
    // Check if it's a double-dash flag (--) or single-dash flag (-)
    if (current() == '-') {
        advance(); // Skip the second dash
        while (isalnum(current()) || current() == '_' || current() == '-') {
            flag += current();
            advance();
        }
        return Token(TokenType::FLAG, "--" + flag, startLine, startColumn);
    } else {
        // Single-dash flag (e.g., -p, -d)
        while (isalnum(current())) {
            flag += current();
            advance();
        }
        return Token(TokenType::FLAG, "-" + flag, startLine, startColumn);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (position < source.length()) {
        skipWhitespace();
        skipComment();
        
        if (position >= source.length()) break;
        
        char c = current();
        int currentLine = line;
        int currentColumn = column;
        
        if (c == '\n') {
            tokens.emplace_back(TokenType::NEWLINE, "\\n", currentLine, currentColumn);
            advance();
        }
        else if (isdigit(c)) {
            tokens.push_back(readNumber());
        }
        else if (c == '"') {
            tokens.push_back(readString());
        }
        else if (isalpha(c) || c == '_') {
            tokens.push_back(readIdentifier());
        }
        else if (c == '-' && peek() == '-') {
            tokens.push_back(readFlag());
        }
        else if (c == '-' && (isalnum(peek(1)) || peek(1) == '_')) {
            tokens.push_back(readFlag());
        }
        else if (c == '=') {
            advance();
            if (current() == '=') {
                advance();
                tokens.emplace_back(TokenType::EQUALS, "==", currentLine, currentColumn);
            } else {
                tokens.emplace_back(TokenType::ASSIGN, "=", currentLine, currentColumn);
            }
        }
        else if (c == '+') {
            advance();
            tokens.emplace_back(TokenType::PLUS, "+", currentLine, currentColumn);
        }
        else if (c == '-') {
            advance();
            tokens.emplace_back(TokenType::MINUS, "-", currentLine, currentColumn);
        }
        else if (c == '*') {
            advance();
            tokens.emplace_back(TokenType::MULTIPLY, "*", currentLine, currentColumn);
        }
        else if (c == '/') {
            advance();
            tokens.emplace_back(TokenType::DIVIDE, "/", currentLine, currentColumn);
        }
        else if (c == '(') {
            advance();
            tokens.emplace_back(TokenType::LPAREN, "(", currentLine, currentColumn);
        }
        else if (c == ')') {
            advance();
            tokens.emplace_back(TokenType::RPAREN, ")", currentLine, currentColumn);
        }
        else if (c == '>') {
            advance();
            if (current() == '=') {
                advance();
                tokens.emplace_back(TokenType::GREATER_EQUAL, ">=", currentLine, currentColumn);
            } else {
                tokens.emplace_back(TokenType::GREATER_THAN, ">", currentLine, currentColumn);
            }
        }
        else if (c == '<') {
            advance();
            if (current() == '=') {
                advance();
                tokens.emplace_back(TokenType::LESS_EQUAL, "<=", currentLine, currentColumn);
            } else {
                tokens.emplace_back(TokenType::LESS_THAN, "<", currentLine, currentColumn);
            }
        }
        else if (c == '&') {
            advance();
            if (current() == '&') {
                advance();
                tokens.emplace_back(TokenType::AND, "&&", currentLine, currentColumn);
            } else {
                tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c), currentLine, currentColumn);
            }
        }
        else if (c == '|') {
            advance();
            if (current() == '|') {
                advance();
                tokens.emplace_back(TokenType::OR, "||", currentLine, currentColumn);
            } else {
                tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c), currentLine, currentColumn);
            }
        }
        else if (c == '!') {
            advance();
            if (current() == '=') {
                advance();
                tokens.emplace_back(TokenType::NOT_EQUAL, "!=", currentLine, currentColumn);
            } else {
                tokens.emplace_back(TokenType::NOT, "!", currentLine, currentColumn);
            }
        }
        else if (c == ':') {
            advance();
            tokens.emplace_back(TokenType::COLON, ":", currentLine, currentColumn);
        }
        else if (c == ',') {
            advance();
            tokens.emplace_back(TokenType::COMMA, ",", currentLine, currentColumn);
        }
        else {
            advance();
            tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c), currentLine, currentColumn);
        }
    }
    
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column);
    return tokens;
}

void Lexer::printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout << "Token(" << token.line << ":" << token.column << ") ";
        
        switch (token.type) {
            case TokenType::SET: std::cout << "SET"; break;
            case TokenType::IF: std::cout << "IF"; break;
            case TokenType::ELSE: std::cout << "ELSE"; break;
            case TokenType::RUN: std::cout << "RUN"; break;
            case TokenType::PRINT: std::cout << "PRINT"; break;
            case TokenType::DO: std::cout << "DO"; break;
            case TokenType::OUT: std::cout << "OUT"; break;
            case TokenType::RES: std::cout << "RES"; break;
            case TokenType::STOP: std::cout << "STOP"; break;
            case TokenType::WHILE: std::cout << "WHILE"; break;
            case TokenType::FOR: std::cout << "FOR"; break;
            case TokenType::EXEC: std::cout << "EXEC"; break;
            case TokenType::FLAG: std::cout << "FLAG(" << token.value << ")"; break;
            case TokenType::NUMBER: std::cout << "NUMBER(" << token.value << ")"; break;
            case TokenType::STRING: std::cout << "STRING(\"" << token.value << "\")"; break;
            case TokenType::BOOLEAN: std::cout << "BOOLEAN(" << token.value << ")"; break;
            case TokenType::IDENTIFIER: std::cout << "IDENTIFIER(" << token.value << ")"; break;
            case TokenType::EQUALS: std::cout << "EQUALS"; break;
            case TokenType::GREATER_THAN: std::cout << "GREATER_THAN"; break;
            case TokenType::GREATER_EQUAL: std::cout << "GREATER_EQUAL"; break;
            case TokenType::LESS_THAN: std::cout << "LESS_THAN"; break;
            case TokenType::LESS_EQUAL: std::cout << "LESS_EQUAL"; break;
            case TokenType::NOT_EQUAL: std::cout << "NOT_EQUAL"; break;
            case TokenType::AND: std::cout << "AND"; break;
            case TokenType::OR: std::cout << "OR"; break;
            case TokenType::NOT: std::cout << "NOT"; break;
            case TokenType::ASSIGN: std::cout << "ASSIGN"; break;
            case TokenType::PLUS: std::cout << "PLUS"; break;
            case TokenType::MINUS: std::cout << "MINUS"; break;
            case TokenType::MULTIPLY: std::cout << "MULTIPLY"; break;
            case TokenType::DIVIDE: std::cout << "DIVIDE"; break;
            case TokenType::LPAREN: std::cout << "LPAREN"; break;
            case TokenType::RPAREN: std::cout << "RPAREN"; break;
            case TokenType::COLON: std::cout << "COLON"; break;
            case TokenType::COMMA: std::cout << "COMMA"; break;
            case TokenType::NEWLINE: std::cout << "NEWLINE"; break;
            case TokenType::EOF_TOKEN: std::cout << "EOF"; break;
            case TokenType::UNKNOWN: std::cout << "UNKNOWN(" << token.value << ")"; break;
        }
        
        std::cout << std::endl;
    }
}