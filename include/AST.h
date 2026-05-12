#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <variant>

// Forward declarations
class NodeVisitor;

// Base AST Node
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(NodeVisitor& visitor) = 0;
};

// Expression nodes
class NumberNode : public ASTNode {
public:
    double value;
    
    NumberNode(double val) : value(val) {}
    void accept(NodeVisitor& visitor) override;
};

class StringNode : public ASTNode {
public:
    std::string value;
    
    StringNode(const std::string& val) : value(val) {}
    void accept(NodeVisitor& visitor) override;
};

class BooleanNode : public ASTNode {
public:
    bool value;
    
    BooleanNode(bool val) : value(val) {}
    void accept(NodeVisitor& visitor) override;
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    
    IdentifierNode(const std::string& n) : name(n) {}
    void accept(NodeVisitor& visitor) override;
};

class BinaryOpNode : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    
    BinaryOpNode(const std::string& op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    void accept(NodeVisitor& visitor) override;
};

class UnaryOpNode : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> operand;
    
    UnaryOpNode(const std::string& op, std::unique_ptr<ASTNode> operand)
        : op(op), operand(std::move(operand)) {}
    void accept(NodeVisitor& visitor) override;
};

class FunctionCallNode : public ASTNode {
public:
    std::string functionName;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    
    FunctionCallNode(const std::string& name, std::vector<std::unique_ptr<ASTNode>> args)
        : functionName(name), arguments(std::move(args)) {}
    void accept(NodeVisitor& visitor) override;
};

// Statement nodes
class SetNode : public ASTNode {
public:
    std::string variableName;
    std::unique_ptr<ASTNode> value;
    
    SetNode(const std::string& name, std::unique_ptr<ASTNode> val)
        : variableName(name), value(std::move(val)) {}
    void accept(NodeVisitor& visitor) override;
};

class DoNode : public ASTNode {
public:
    std::string variableName;
    std::unique_ptr<ASTNode> value;
    
    DoNode(const std::string& name, std::unique_ptr<ASTNode> val)
        : variableName(name), value(std::move(val)) {}
    void accept(NodeVisitor& visitor) override;
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> thenBranch;
    std::vector<std::unique_ptr<ASTNode>> elseBranch;
    
    IfNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> thenStmts,
           std::vector<std::unique_ptr<ASTNode>> elseStmts)
        : condition(std::move(cond)), thenBranch(std::move(thenStmts)), 
          elseBranch(std::move(elseStmts)) {}
    void accept(NodeVisitor& visitor) override;
};

class PrintNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> value;
    
    PrintNode(std::unique_ptr<ASTNode> val) : value(std::move(val)) {}
    void accept(NodeVisitor& visitor) override;
};

class OutNode : public ASTNode {
public:
    OutNode() = default;
    void accept(NodeVisitor& visitor) override;
};

class ResNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> value;
    
    ResNode(std::unique_ptr<ASTNode> val) : value(std::move(val)) {}
    void accept(NodeVisitor& visitor) override;
};

class StopNode : public ASTNode {
public:
    StopNode() = default;
    void accept(NodeVisitor& visitor) override;
};

class RunNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> functionCall;
    
    RunNode(std::unique_ptr<ASTNode> call) : functionCall(std::move(call)) {}
    void accept(NodeVisitor& visitor) override;
};

class FunctionDefNode : public ASTNode {
public:
    std::string functionName;
    std::vector<std::unique_ptr<ASTNode>> body;
    
    FunctionDefNode(const std::string& name, std::vector<std::unique_ptr<ASTNode>> stmts)
        : functionName(name), body(std::move(stmts)) {}
    void accept(NodeVisitor& visitor) override;
};

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    BlockNode(std::vector<std::unique_ptr<ASTNode>> stmts) : statements(std::move(stmts)) {}
    void accept(NodeVisitor& visitor) override;
};

// Visitor pattern for AST traversal
class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(StringNode& node) = 0;
    virtual void visit(BooleanNode& node) = 0;
    virtual void visit(IdentifierNode& node) = 0;
    virtual void visit(BinaryOpNode& node) = 0;
    virtual void visit(UnaryOpNode& node) = 0;
    virtual void visit(FunctionCallNode& node) = 0;
    virtual void visit(SetNode& node) = 0;
    virtual void visit(DoNode& node) = 0;
    virtual void visit(IfNode& node) = 0;
    virtual void visit(PrintNode& node) = 0;
    virtual void visit(OutNode& node) = 0;
    virtual void visit(ResNode& node) = 0;
    virtual void visit(StopNode& node) = 0;
    virtual void visit(RunNode& node) = 0;
    virtual void visit(FunctionDefNode& node) = 0;
    virtual void visit(BlockNode& node) = 0;
};

// Accept implementations
inline void NumberNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void StringNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void BooleanNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void IdentifierNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void BinaryOpNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void UnaryOpNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void FunctionCallNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void SetNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void DoNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void IfNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void PrintNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void OutNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void ResNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void StopNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void RunNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void FunctionDefNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }
inline void BlockNode::accept(NodeVisitor& visitor) { visitor.visit(*this); }

#endif // AST_H
