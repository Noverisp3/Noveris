#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "AST.h"
#include <unordered_map>
#include <variant>

using Value = std::variant<double, std::string, bool, std::monostate>;

class Interpreter : public NodeVisitor {
private:
    std::unordered_map<std::string, Value> variables;
    std::unordered_map<std::string, std::unique_ptr<FunctionDefNode>> functions;
    Value returnValue;
    bool hasReturnValue = false;
    bool shouldBreak = false;
    bool shouldStop = false;
    
    std::string valueToString(const Value& val);
    std::string processEscapeSequences(const std::string& input);
    double valueToDouble(const Value& val);
    bool valueToBool(const Value& val);
    
public:
    Interpreter() = default;
    
    void interpret(BlockNode* block);
    
    void visit(NumberNode& node) override;
    void visit(StringNode& node) override;
    void visit(BooleanNode& node) override;
    void visit(IdentifierNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(UnaryOpNode& node) override;
    void visit(FunctionCallNode& node) override;
    void visit(SetNode& node) override;
    void visit(DoNode& node) override;
    void visit(IfNode& node) override;
    void visit(PrintNode& node) override;
    void visit(OutNode& node) override;
    void visit(ResNode& node) override;
    void visit(StopNode& node) override;
    void visit(RunNode& node) override;
    void visit(FunctionDefNode& node) override;
    void visit(BlockNode& node) override;
};

#endif // INTERPRETER_H
