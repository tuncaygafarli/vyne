#include "ast.h"
#include <stdexcept>

Value NumberNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    return Value(value);
}

Value VariableNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string targetGroup;
    
    if (specificGroup.empty()) {
        targetGroup = currentGroup;
    } else {
        targetGroup = "global";
        for (const auto& g : specificGroup) {
            targetGroup += "." + g;
        }
    }

    if (env.count(targetGroup) && env[targetGroup].count(name)) {
        return env[targetGroup][name];
    }

    throw std::runtime_error("Variable '" + name + "' not found in " + targetGroup);
}

Value AssignmentNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value val = rhs->evaluate(env, currentGroup);
    
    std::string targetGroup;

    if (scopePath.empty()) {
        targetGroup = currentGroup;
    } else {
        targetGroup = scopePath[0];
        for (size_t i = 1; i < scopePath.size(); ++i) {
            targetGroup += "." + scopePath[i];
        }

        if (currentGroup == "global" && env.find(targetGroup) == env.end()) {
            std::string globalPath = "global." + targetGroup;
            if (env.find(globalPath) != env.end()) {
                targetGroup = globalPath;
            }
        }
    }

    env[targetGroup][identifier] = val; 
    return val;
}

Value GroupNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string nextGroup = currentGroup + "." + groupName;
    
    for (const auto& stmt : statements) {
        stmt->evaluate(env, nextGroup);
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

Value IndexAccessNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string targetGroup = resolvePath(scope, currentGroup);

    if (!env.count(targetGroup) || !env[targetGroup].count(name)) {
        throw std::runtime_error("Array '" + name + "' not found.");
    }

    Value& arrayVal = env[targetGroup][name];

    if (arrayVal.type != Value::ARRAY) {
        throw std::runtime_error("Type Error: '" + name + "' is not an array.");
    }

    Value idxVal = index->evaluate(env, currentGroup);
    
    if (idxVal.type != Value::NUMBER) {
        throw std::runtime_error("Index must be a number.");
    }

    int i = static_cast<int>(idxVal.number);

    if (i < 0 || i >= static_cast<int>(arrayVal.list.size())) {
        throw std::runtime_error("Index out of bounds: " + std::to_string(i));
    }

    return arrayVal.list[i]; 
}

Value MethodCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    VariableNode* var = dynamic_cast<VariableNode*>(receiver.get());

    if(var){
        std::string targetGroup = resolvePath(var->getScope(), currentGroup);
        Value& target = env[targetGroup][var->getName()];

        if(methodName == "size"){
            if(target.type != Value::ARRAY) throw std::runtime_error("Compilation error : Called method size() on non-array!");

            return Value(static_cast<double>(target.list.size()));
        }

        if (methodName == "push"){
            if(target.type != Value::ARRAY) throw std::runtime_error("Compilation error : Called method push() on non-array!");

            Value val = arguments[0]->evaluate(env, currentGroup);
            target.list.emplace_back(val);
            return val;
        }
    } else {
        Value temp = receiver->evaluate(env, currentGroup);
        if (methodName == "size") {
            if (temp.type != Value::ARRAY) throw std::runtime_error("size() called on non-array.");
            return Value(static_cast<double>(temp.list.size()));
        }

        throw std::runtime_error("Cannot modify a literal array!");
    }
    
    throw std::runtime_error("Unknown method: " + methodName);
}

// helpers
std::string resolvePath(std::vector<std::string> scope, std::string currentGroup) {
    std::string targetGroup;
    
    if (scope.empty()) {
        targetGroup = currentGroup;
    } else {
        targetGroup = "global";
        for (const auto& g : scope) {
            targetGroup += "." + g;
        }
    }

    return targetGroup;
}