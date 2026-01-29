#include "ast.h"
#include "../../modules/vcore/vcore.h"

Value NumberNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    return Value(value);
}

Value VariableNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string targetGroup = specificGroup.empty() ? currentGroup : "global";
    if (!specificGroup.empty()) {
        for (const auto& g : specificGroup) targetGroup += "." + g;
    }

    auto groupIt = env.find(targetGroup);
    if (groupIt != env.end()) {
        auto varIt = groupIt->second.find(nameId);
        if (varIt != groupIt->second.end()) return varIt->second;
    }

    if (targetGroup != "global") {
        auto globalIt = env.find("global");
        if (globalIt != env.end()) {
            auto varIt = globalIt->second.find(nameId);
            if (varIt != globalIt->second.end()) return varIt->second;
        }
    }

    throw std::runtime_error("Runtime Error: Variable ID '" + std::to_string(nameId) + "' not found.");
}

Value AssignmentNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value val = rhs->evaluate(env, currentGroup);
    std::string targetGroup = resolvePath(scopePath, currentGroup);

    auto& table = env[targetGroup]; 
    auto it = table.find(identifierId);
    
    if (it != table.end() && it->second.isReadOnly) {
        throw std::runtime_error("Runtime Error: Cannot reassign read-only '" + originalName + "'");
    }

    table[identifierId] = val; 
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

    if (l.getType() == Value::STRING && r.getType() == Value::STRING) {
        if (op == TokenType::Add) return Value(l.asString() + r.asString());
        throw std::runtime_error("Type Error: Only '+' allowed for strings.");
    }

    if (l.getType() == Value::ARRAY && r.getType() == Value::ARRAY) {
        if (op == TokenType::Add) {
            Value result = l;
            result.asList().insert(result.asList().end(), r.asList().begin(), r.asList().end());
            return result;
        }
    }

    switch (op) {
        case TokenType::Add: return Value(l.asNumber() + r.asNumber());
        case TokenType::Substract: return Value(l.asNumber() - r.asNumber());
        case TokenType::Multiply: return Value(l.asNumber() * r.asNumber());
        case TokenType::Division:
            if (r.asNumber() == 0) throw std::runtime_error("Division by zero!");
            return Value(l.asNumber() / r.asNumber());
        case TokenType::Smaller: return Value(l.asNumber() < r.asNumber());
        case TokenType::Greater: return Value(l.asNumber() > r.asNumber());
        case TokenType::Double_Equals: return Value(l == r);
        default: return Value(0.0);
    }
}

Value ArrayNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::vector<Value> results;
    for (const auto& node : elements) results.emplace_back(node->evaluate(env, currentGroup));
    return Value(results);
}

Value BuiltInCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::vector<Value> argValues;
    for (auto& arg : arguments) argValues.push_back(arg->evaluate(env, currentGroup));

    if (funcName == "log") {
        if (!argValues.empty()) { argValues[0].print(std::cout); std::cout << std::endl; }
        return Value();
    }
    if (funcName == "type") {
        int type = argValues[0].getType();
        switch(type) {
            case Value::NUMBER: return Value("number");
            case Value::STRING: return Value("string");
            case Value::ARRAY:  return Value("array");
            case Value::FUNCTION: return Value("function");
            default: return Value("unknown");
        }
    }
    throw std::runtime_error("Unknown built-in: " + funcName);
}

Value IndexAccessNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string targetGroup = resolvePath(scope, currentGroup);
    auto it = env[targetGroup].find(nameId);

    if (it == env[targetGroup].end()) throw std::runtime_error("Array not found.");
    Value& arrayVal = it->second;

    Value idxVal = index->evaluate(env, currentGroup);
    int i = static_cast<int>(idxVal.asNumber());
    return arrayVal.asList().at(i); 
}

#include "ast.h"
#include <random>

Value FunctionNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value funcValue(parameterIds, body);

    env[currentGroup][funcNameId] = funcValue; 

    return funcValue;
}

Value FunctionCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    auto it = env["global"].find(funcNameId);
    
    if (it == env["global"].end()) {
        throw std::runtime_error("Runtime Error: " + originalName + " is not defined in global scope.");
    }

    Value funcVal = it->second;

    if (funcVal.getType() != Value::FUNCTION) {
        throw std::runtime_error("Type Error: " + originalName + " is not a function.");
    }

    std::vector<Value> evaluatedArgs;
    for (const auto& arg : arguments) {
        evaluatedArgs.emplace_back(arg->evaluate(env, currentGroup));
    }

    std::string localScope = "call_" + originalName + "_" + std::to_string(rand());

    auto& params = funcVal.asFunction()->params;

    if (params.size() != evaluatedArgs.size()) {
        throw std::runtime_error("Argument Error: Argument count mismatch on function call " + originalName);
    }
    
    for (size_t i = 0; i < params.size(); ++i) {
        env[localScope][params[i]] = evaluatedArgs[i];
    }

    Value result;
    try {
        for (const auto& bodyNode : funcVal.asFunction()->body) {
            result = bodyNode->evaluate(env, localScope);
        }
    } catch (const ReturnException& e) {
        result = e.value; 
    }

    env.erase(localScope);

    return result;
}

Value ReturnNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    throw ReturnException{expression->evaluate(env, currentGroup)};
}

Value MethodCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value receiverVal = receiver->evaluate(env, currentGroup);
    
    uint32_t methodId = StringPool::instance().intern(methodName);

    if (receiverVal.getType() == Value::MODULE) {
        std::string modName = receiverVal.asModule();
        std::string modPath = "global." + modName;

        if (env.count(modPath) && env[modPath].count(methodId)) {
            Value& funcVal = env[modPath][methodId];
            
            std::vector<Value> argValues;
            for (auto& arg : arguments) {
                argValues.emplace_back(arg->evaluate(env, currentGroup));
            }

            try {
                if (funcVal.getType() == Value::FUNCTION && funcVal.asFunction()->isNative) {
                    return funcVal.asFunction()->nativeFn(argValues);
                }
            } catch (const std::exception& e) {
                throw std::runtime_error("Compilation Error : line " + std::to_string(lineNumber) + ": " + e.what());
            }         
        }
        throw std::runtime_error("Module Error: Method '" + methodName + "' not found in module " + modName);
    }

    // --- ARRAY METHODS ---
    if (receiverVal.getType() == Value::ARRAY) {
        VariableNode* var = dynamic_cast<VariableNode*>(receiver.get());
        if (!var) throw std::runtime_error("Runtime Error: Cannot call methods on anonymous arrays at line " + std::to_string(lineNumber));

        std::string targetGroup = resolvePath(var->getScope(), currentGroup);
        Value& target = env[targetGroup][var->getNameId()];

        if (methodName == "size") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method size() on non-array at line " + std::to_string(lineNumber));
            return Value(static_cast<double>(target.asList().size()));
        }

        if (methodName == "push") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method push() on non-array at line " + std::to_string(lineNumber));
            Value val = arguments[0]->evaluate(env, currentGroup);
            target.asList().emplace_back(val);
            return val;
        }

        if (methodName == "pop") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method pop() on non-array at line " + std::to_string(lineNumber));
            if (target.asList().empty()) throw std::runtime_error("Index Error: pop() from empty array at line " + std::to_string(lineNumber));
            if (!arguments.empty()) throw std::runtime_error("Argument Error: pop() expects 0 arguments, but got " + std::to_string(arguments.size()) + " at line " + std::to_string(lineNumber));

            target.asList().pop_back();
            return Value(true);
        }

        if (methodName == "delete") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method delete() on non-array at line " + std::to_string(lineNumber));
            if (arguments.size() != 1) throw std::runtime_error("Argument Error: delete() expects exactly 1 argument, but got " + std::to_string(arguments.size()) + " instead at line " + std::to_string(lineNumber));

            Value val = arguments[0]->evaluate(env, currentGroup);
            auto it = std::find(target.asList().begin(), target.asList().end(), val);
            if (it == std::end(target.asList())) throw std::runtime_error("Value error : Could not find given value in array!");
            target.asList().erase(it);
            return Value(true);
        }

        if (methodName == "sort") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: sort() called on non-array at line " + std::to_string(lineNumber));
            if (!arguments.empty()) throw std::runtime_error("Argument Error: sort() expects 0 arguments.");
            for (auto& el : target.asList()) {
                if (el.getType() != Value::NUMBER) throw std::runtime_error("Value Error: Cannot sort string values at line " + std::to_string(lineNumber));
            }
            std::sort(target.asList().begin(), target.asList().end());
            return Value(target); 
        }

        if (methodName == "place_all") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: place_all() called on non-array at line " + std::to_string(lineNumber));
            if (arguments.size() > 2) throw std::runtime_error("Argument Error: place_all() expects 2 arguments, but got " + std::to_string(arguments.size()) + " instead at line " + std::to_string(lineNumber));
            Value element = arguments[0]->evaluate(env, currentGroup);
            Value count = arguments[1]->evaluate(env, currentGroup);
            std::vector<Value> arr;
            for (size_t i = 0; i < count.asNumber(); i++) arr.emplace_back(element);
            return Value(arr);
        }

        if (methodName == "reverse") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: reverse() called on non-array at line " + std::to_string(lineNumber));
            if (arguments.size() > 0) throw std::runtime_error("Argument Error: reverse() expects 0 arguments, but got " + std::to_string(arguments.size()) + " instead at line " + std::to_string(lineNumber));
            std::reverse(target.asList().begin(), target.asList().end());
            return Value(target);
        }

        if (methodName == "clear") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: clear() called on non-array at line " + std::to_string(lineNumber));
            if (arguments.size() > 0) throw std::runtime_error("Argument Error: clear() expects 0 arguments, but got " + std::to_string(arguments.size()) + " instead at line " + std::to_string(lineNumber));
            target.asList().clear();
            return Value(target);
        }
    }
    
    throw std::runtime_error("Unknown method: " + methodName);
}

Value WhileNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value lastResult;
    while (condition->evaluate(env, currentGroup).isTruthy()) {
        try {
            lastResult = body->evaluate(env, currentGroup);
        } catch (const BreakException&) { break; }
          catch (const ContinueException&) { continue; }
    }
    return lastResult; 
}

Value IfNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    if(condition->evaluate(env, currentGroup).isTruthy()){
        return body->evaluate(env, currentGroup);
    }
    return Value();
}

Value BlockNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value lastValue;
    for (const auto& statement : statements) lastValue = statement->evaluate(env, currentGroup);
    return lastValue; 
}

Value ModuleNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    if (originalName == "vcore") {
        setupVCore(env, StringPool::instance());
    } 

    env[currentGroup][moduleId] = Value(originalName, true); 
    
    return env[currentGroup][moduleId];
}

std::string resolvePath(std::vector<std::string> scope, std::string currentGroup) {
    if (scope.empty()) return currentGroup;
    std::string targetGroup = "global";
    for (const auto& g : scope) targetGroup += "." + g;
    return targetGroup;
}