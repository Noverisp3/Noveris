#include "../include/Lexer.h"
#include "../include/Parser.h"
#include "../include/Interpreter.h"
#include "../include/AST.h"
#include <iostream>
#include <fstream>
#include <string>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: noveris [-v|--verbose] <filename.nv>" << std::endl;
        return 1;
    }
    
    bool verbose = false;
    std::string filename;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (filename.empty()) {
            filename = arg;
        } else {
            std::cerr << "Usage: noveris [-v|--verbose] <filename.nv>" << std::endl;
            return 1;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Usage: noveris [-v|--verbose] <filename.nv>" << std::endl;
        return 1;
    }
    
    try {
        // Read source file
        std::string source = readFile(filename);
        
        // Lexical analysis
        if (verbose) {
            std::cout << "=== Lexical Analysis ===" << std::endl;
        }
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        if (verbose) {
            std::cout << "Tokenized " << tokens.size() << " tokens" << std::endl;
        }
        
        // Parsing
        if (verbose) {
            std::cout << "\n=== Parsing ===" << std::endl;
        }
        Parser parser(tokens);
        auto ast = parser.parse();
        if (verbose) {
            std::cout << "Parsing completed" << std::endl;
        }
        
        // Interpretation
        if (verbose) {
            std::cout << "\n=== Execution ===" << std::endl;
        }
        Interpreter interpreter;
        interpreter.interpret(ast.get());
        if (verbose) {
            std::cout << "" << std::endl;
            std::cout << "Execution completed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
