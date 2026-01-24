#include "ast.h"
#include <stdexcept>

Value NumberNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    return Value(value);
}

Value VariableNode::evaluate(SymbolContainer& forest, std::string currentGroup) const {
    std::string targetGroup;
    
    if (specificGroup.empty()) {
        targetGroup = currentGroup;
    } else {
        targetGroup = "global";
        for (const auto& g : specificGroup) {
            targetGroup += "." + g;
        }
    }

    if (forest.count(targetGroup) && forest[targetGroup].count(name)) {
        return forest[targetGroup][name];
    }

    throw std::runtime_error("Variable '" + name + "' not found in " + targetGroup);
}

Value AssignmentNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value val = rhs->evaluate(env, currentGroup);
    
    env[currentGroup][identifier] = val; 
    return val;
}

Value GroupNode::evaluate(SymbolContainer& forest, std::string currentGroup) const {
    std::string nextGroup = currentGroup + "." + groupName;
    
    for (const auto& stmt : statements) {
        stmt->evaluate(forest, nextGroup);
    }
    return Value();
}

Value BinOpNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value l = left->evaluate(env, currentGroup);
    Value r = right->evaluate(env, currentGroup);

    if (l.type == Value::STRING && r.type == Value::STRING) {
        if (op == '+') return Value(l.text + r.text);
        throw std::runtime_error("Type Error: Only '+' allowed for strings.");
    }

    if ((l.type == Value::STRING) != (r.type == Value::STRING)) {
        throw std::runtime_error("Type Error: Cannot mix strings and numbers!");
    }

    switch (op) {
    case '+': return Value(l.number + r.number);
    case '-': return Value(l.number - r.number);
    case '*': return Value(l.number * r.number);
    case '/':
        if (r.number == 0) throw std::runtime_error("Division by zero!");
        return Value(l.number / r.number);
    case '<': return Value(l.number < r.number);
    case '>': return Value(l.number > r.number);
    default:  return Value(0.0);
    }
}

Value PrintNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value val = expression->evaluate(env, currentGroup);
    val.print(std::cout);
    return val;
}

Value ArrayNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::vector<Value> results;

    for (const auto& node : elements){
        results.emplace_back(node->evaluate(env, currentGroup));
    }

    return Value(results);
}