#include "../include/Interpreter.h"
#include <iostream>
#include <stdexcept>

void Interpreter::throwAt(const ASTNode& node, const std::string& message) {
    throw std::runtime_error(node.sourcePrefix() + message);
}

void Interpreter::interpret(BlockNode* block) {
    for (auto& stmt : block->statements) {
        if (shouldStop) break;
        // Ensure clean state before each statement
        shouldBreak = false;
        hasReturnValue = false;
        stmt->accept(*this);
        // Reset flags after each statement to prevent state carryover
        shouldBreak = false;
        hasReturnValue = false;
    }
}

std::string Interpreter::valueToString(const Value& val) {
    if (std::holds_alternative<double>(val)) {
        return std::to_string(std::get<double>(val));
    } else if (std::holds_alternative<std::string>(val)) {
        return std::get<std::string>(val);
    } else if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val) ? "true" : "false";
    }
    return "null";
}

std::string Interpreter::processEscapeSequences(const std::string& input) {
    std::string result;
    result.reserve(input.length() * 2); // Reserve space for potential expansions
    
    for (size_t i = 0; i < input.length(); ++i) {
        if (i + 1 < input.length() && input[i] == '\\' && i + 1 < input.length()) {
            // Handle escape sequences
            switch (input[i + 1]) {
                case 'n':
                    result.push_back('\n');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case '\\':
                    result.push_back('\\');
                    ++i; // Skip the second backslash
                    continue; // Skip the rest of the loop
                case '"':
                    result.push_back('"');
                    break;
                case '\'':
                    result.push_back('\'');
                    break;
                default:
                    // Unknown escape sequence, keep both characters
                    result.push_back(input[i]);
                    result.push_back(input[i + 1]);
                    break;
            }
            ++i; // Skip the escape character
        } else {
            result.push_back(input[i]);
        }
    }
    
    return result;
}

double Interpreter::valueToDouble(const Value& val) {
    if (std::holds_alternative<double>(val)) {
        return std::get<double>(val);
    } else if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val) ? 1.0 : 0.0;
    }
    throw std::runtime_error("Cannot convert value to number");
}

bool Interpreter::valueToBool(const Value& val) {
    if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val);
    } else if (std::holds_alternative<double>(val)) {
        return std::get<double>(val) != 0.0;
    } else if (std::holds_alternative<std::string>(val)) {
        return !std::get<std::string>(val).empty();
    }
    return false;
}

void Interpreter::visit(NumberNode& node) {
    returnValue = node.value;
    hasReturnValue = true;
}

void Interpreter::visit(StringNode& node) {
    returnValue = node.value;
    hasReturnValue = true;
}

void Interpreter::visit(BooleanNode& node) {
    returnValue = node.value;
    hasReturnValue = true;
}

void Interpreter::visit(IdentifierNode& node) {
    if (variables.find(node.name) != variables.end()) {
        returnValue = variables[node.name];
        hasReturnValue = true;
    } else {
        throwAt(node, "Undefined variable: " + node.name);
    }
}

void Interpreter::visit(BinaryOpNode& node) {
    if (node.op == "&&") {
        node.left->accept(*this);
        Value leftVal = returnValue;
        hasReturnValue = false;
        if (!valueToBool(leftVal)) {
            returnValue = false;
            hasReturnValue = true;
            return;
        }
        node.right->accept(*this);
        Value rightVal = returnValue;
        hasReturnValue = false;
        returnValue = valueToBool(rightVal);
        hasReturnValue = true;
        return;
    }
    if (node.op == "||") {
        node.left->accept(*this);
        Value leftVal = returnValue;
        hasReturnValue = false;
        if (valueToBool(leftVal)) {
            returnValue = true;
            hasReturnValue = true;
            return;
        }
        node.right->accept(*this);
        Value rightVal = returnValue;
        hasReturnValue = false;
        returnValue = valueToBool(rightVal);
        hasReturnValue = true;
        return;
    }

    node.left->accept(*this);
    Value leftVal = returnValue;
    hasReturnValue = false;
    
    node.right->accept(*this);
    Value rightVal = returnValue;
    hasReturnValue = false;
    
    if (node.op == "+") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) + std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal)) {
            returnValue = std::get<std::string>(leftVal) + valueToString(rightVal);
        } else {
            returnValue = valueToString(leftVal) + valueToString(rightVal);
        }
    } else if (node.op == "-") {
        try {
            returnValue = valueToDouble(leftVal) - valueToDouble(rightVal);
        } catch (const std::runtime_error&) {
            throwAt(node, "Cannot convert value to number");
        }
    } else if (node.op == "*") {
        try {
            returnValue = valueToDouble(leftVal) * valueToDouble(rightVal);
        } catch (const std::runtime_error&) {
            throwAt(node, "Cannot convert value to number");
        }
    } else if (node.op == "/") {
        double dividend;
        double divisor;
        try {
            dividend = valueToDouble(leftVal);
            divisor = valueToDouble(rightVal);
        } catch (const std::runtime_error&) {
            throwAt(node, "Cannot convert value to number");
        }
        if (divisor == 0.0) {
            throwAt(node, "Division by zero");
        }
        returnValue = dividend / divisor;
    } else if (node.op == "=") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) == std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            returnValue = std::get<std::string>(leftVal) == std::get<std::string>(rightVal);
        } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
            returnValue = std::get<bool>(leftVal) == std::get<bool>(rightVal);
        } else {
            returnValue = false;
        }
    } else if (node.op == ">") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) > std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            returnValue = std::get<std::string>(leftVal) > std::get<std::string>(rightVal);
        } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
            returnValue = std::get<bool>(leftVal) > std::get<bool>(rightVal);
        } else {
            // Mixed type comparison: convert both to strings and compare
            returnValue = valueToString(leftVal) > valueToString(rightVal);
        }
    } else if (node.op == "<") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) < std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            returnValue = std::get<std::string>(leftVal) < std::get<std::string>(rightVal);
        } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
            returnValue = std::get<bool>(leftVal) < std::get<bool>(rightVal);
        } else {
            // Mixed type comparison: convert both to strings and compare
            returnValue = valueToString(leftVal) < valueToString(rightVal);
        }
    } else if (node.op == ">=") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) >= std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            returnValue = std::get<std::string>(leftVal) >= std::get<std::string>(rightVal);
        } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
            returnValue = std::get<bool>(leftVal) >= std::get<bool>(rightVal);
        } else {
            // Mixed type comparison: convert both to strings and compare
            returnValue = valueToString(leftVal) >= valueToString(rightVal);
        }
    } else if (node.op == "<=") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) <= std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            returnValue = std::get<std::string>(leftVal) <= std::get<std::string>(rightVal);
        } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
            returnValue = std::get<bool>(leftVal) <= std::get<bool>(rightVal);
        } else {
            // Mixed type comparison: convert both to strings and compare
            returnValue = valueToString(leftVal) <= valueToString(rightVal);
        }
    } else if (node.op == "!=") {
        if (std::holds_alternative<double>(leftVal) && std::holds_alternative<double>(rightVal)) {
            returnValue = std::get<double>(leftVal) != std::get<double>(rightVal);
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            returnValue = std::get<std::string>(leftVal) != std::get<std::string>(rightVal);
        } else if (std::holds_alternative<bool>(leftVal) && std::holds_alternative<bool>(rightVal)) {
            returnValue = std::get<bool>(leftVal) != std::get<bool>(rightVal);
        } else {
            // Mixed type comparison: convert both to strings and compare
            returnValue = valueToString(leftVal) != valueToString(rightVal);
        }
    } else {
        throwAt(node, "Unknown binary operator: " + node.op);
    }
    
    hasReturnValue = true;
}

void Interpreter::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
    Value operandValue = returnValue;
    
    if (node.op == "!") {
        bool operandBool = valueToBool(operandValue);
        returnValue = !operandBool;
    } else {
        throwAt(node, "Unknown unary operator: " + node.op);
    }
    
    hasReturnValue = true;
}

void Interpreter::visit(FunctionCallNode& node) {
    if (functions.find(node.functionName) != functions.end()) {
        // Execute the function
        auto& func = functions[node.functionName];
        
        // Save current state
        auto oldVariables = variables;
        auto oldReturnValue = returnValue;
        auto oldHasReturnValue = hasReturnValue;
        auto oldShouldBreak = shouldBreak;
        
        // Reset for function execution
        hasReturnValue = false;
        shouldBreak = false;
        
        // Execute function body
        for (auto& stmt : func->body) {
            stmt->accept(*this);
            if (shouldBreak) break;
        }
        
        // Preserve function return value
        Value functionReturnValue = returnValue;
        bool functionHasReturn = hasReturnValue;
        
        // Restore state
        variables = oldVariables;
        returnValue = oldReturnValue;
        hasReturnValue = oldHasReturnValue;
        shouldBreak = oldShouldBreak;
        
        // Set function return value as current return value
        if (functionHasReturn) {
            returnValue = functionReturnValue;
            hasReturnValue = true;
        }
    } else {
        throwAt(node, "Undefined function: " + node.functionName);
    }
}

void Interpreter::visit(SetNode& node) {
    node.value->accept(*this);
    variables[node.variableName] = returnValue;
    hasReturnValue = false;
}

void Interpreter::visit(DoNode& node) {
    node.value->accept(*this);
    variables[node.variableName] = returnValue;
    hasReturnValue = false;
}

void Interpreter::visit(WhileNode& node) {
    hasReturnValue = false;
    while (true) {
        if (shouldStop) {
            break;
        }
        node.condition->accept(*this);
        if (!valueToBool(returnValue)) {
            break;
        }
        hasReturnValue = false;
        for (auto& stmt : node.body) {
            stmt->accept(*this);
            if (shouldBreak || shouldStop) {
                break;
            }
        }
        if (shouldStop || shouldBreak) {
            break;
        }
    }
    hasReturnValue = false;
}

void Interpreter::visit(ForNode& node) {
    hasReturnValue = false;
    node.startExpr->accept(*this);
    variables[node.loopVar] = returnValue;
    hasReturnValue = false;

    while (!shouldStop) {
        auto it = variables.find(node.loopVar);
        if (it == variables.end()) {
            throwAt(node, "For loop variable not found: " + node.loopVar);
        }
        Value curVal = it->second;

        node.endExpr->accept(*this);
        Value endVal = returnValue;
        hasReturnValue = false;

        double curNum;
        double endNum;
        try {
            curNum = valueToDouble(curVal);
            endNum = valueToDouble(endVal);
        } catch (const std::runtime_error&) {
            throwAt(node, "For loop requires numeric loop variable and end bound");
        }

        if (curNum > endNum) {
            break;
        }

        bool leaveLoop = false;
        for (auto& stmt : node.body) {
            stmt->accept(*this);
            if (shouldBreak || shouldStop) {
                leaveLoop = true;
                break;
            }
        }
        if (leaveLoop || shouldStop || shouldBreak) {
            break;
        }

        it = variables.find(node.loopVar);
        if (it == variables.end()) {
            throwAt(node, "For loop variable missing after body: " + node.loopVar);
        }
        try {
            curNum = valueToDouble(it->second);
        } catch (const std::runtime_error&) {
            throwAt(node, "For loop variable must stay numeric");
        }
        variables[node.loopVar] = curNum + 1.0;
    }
    hasReturnValue = false;
}

void Interpreter::visit(IfNode& node) {
    node.condition->accept(*this);
    bool condition = valueToBool(returnValue);
    
    // Store the return value before executing branches
    Value branchValue;
    bool branchHasValue = false;
    
    if (condition) {
        hasReturnValue = false;
        for (auto& stmt : node.thenBranch) {
            stmt->accept(*this);
            // Capture the return value from the branch if it exists
            if (hasReturnValue) {
                branchValue = returnValue;
                branchHasValue = true;
            }
            // Break immediately if shouldBreak is set (res/out statement encountered)
            if (shouldBreak) break;
        }
    } else {
        hasReturnValue = false;
        for (auto& stmt : node.elseBranch) {
            stmt->accept(*this);
            // Capture the return value from the branch if it exists
            if (hasReturnValue) {
                branchValue = returnValue;
                branchHasValue = true;
            }
            // Break immediately if shouldBreak is set (res/out statement encountered)
            if (shouldBreak) break;
        }
    }
    
    // Set the return value from the executed branch
    if (branchHasValue) {
        returnValue = branchValue;
        hasReturnValue = true;
    } else {
        returnValue = false;
        hasReturnValue = true;
    }
}

void Interpreter::visit(PrintNode& node) {
    node.value->accept(*this);
    Value printedValue = returnValue; // Preserve the value before resetting
    std::cout << valueToString(returnValue) << std::endl;
    hasReturnValue = false;
    returnValue = printedValue; // Restore the preserved value
}

void Interpreter::visit(OutNode& node) {
    (void)node; // Suppress unused parameter warning
    shouldBreak = true;
}

void Interpreter::visit(ResNode& node) {
    node.value->accept(*this);
    hasReturnValue = true;
    shouldBreak = true;
}

void Interpreter::visit(StopNode& node) {
    (void)node; // Suppress unused parameter warning
    shouldStop = true;
    shouldBreak = true;
}

void Interpreter::visit(RunNode& node) {
    node.functionCall->accept(*this);
    // Don't reset hasReturnValue - preserve the function's return value
}

void Interpreter::visit(FunctionDefNode& node) {
    auto newFunc = std::make_unique<FunctionDefNode>(node.functionName,
        std::vector<std::unique_ptr<ASTNode>>());
    for (auto& stmt : node.body) {
        newFunc->body.push_back(stmt->clone());
    }
    functions[node.functionName] = std::move(newFunc);
}

void Interpreter::visit(BlockNode& node) {
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
        if (shouldBreak) break;
    }
}
