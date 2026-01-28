#include "ast.h"

Value NumberNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    return Value(value);
}

Value VariableNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string targetGroup = specificGroup.empty() ? currentGroup : "global";
    
    if (!specificGroup.empty()) {
        for (const auto& g : specificGroup) targetGroup += "." + g;
    }

    if (env.count(targetGroup) && env[targetGroup].count(name)) {
        return env[targetGroup][name];
    }

    if (targetGroup != "global" && env["global"].count(name)) {
        return env["global"][name];
    }

    throw std::runtime_error("Variable '" + name + "' not found in " + targetGroup + " or global.");
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
        throw std::runtime_error("Type Error: Only '+' allowed for strings, numbers and arrays.");
    }

    if ((l.getType() == Value::STRING) != (r.getType() == Value::STRING)) {
        throw std::runtime_error("Type Error: Cannot mix strings and numbers!");
    }

    switch (op) {
    case TokenType::Add : return Value(l.asNumber() + r.asNumber());
    case TokenType::Substract: return Value(l.asNumber() - r.asNumber());
    case TokenType::Multiply: return Value(l.asNumber() * r.asNumber());
    case TokenType::Division:
        if (r.asNumber() == 0) throw std::runtime_error("Division by zero!");
        return Value(l.asNumber() / r.asNumber());
    case TokenType::Smaller: return Value(l.asNumber() < r.asNumber());
    case TokenType::Greater: return Value(l.asNumber() > r.asNumber());
    case TokenType::Double_Equals : return Value(l == r);
    default:  return Value(0.0);
    }
}

Value ArrayNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::vector<Value> results;

    for (const auto& node : elements){
        results.emplace_back(node->evaluate(env, currentGroup));
    }

    return Value(results);
}

Value BuiltInCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::vector<Value> argValues;
    for (auto& arg : arguments) {
        argValues.push_back(arg->evaluate(env, currentGroup));
    }

    if (funcName == "log") {
        if (!argValues.empty()) {
            argValues[0].print(std::cout);
            std::cout << std::endl;
        }
        return Value();
    }
    else if (funcName == "sizeof") {
        if (argValues.empty()) return Value(0.0);
        return Value(static_cast<double>(argValues[0].getBytes()));
    }
    else if (funcName == "string") {
        if (argValues.size() != 1) throw std::runtime_error("Argument Error : string() expects 1 argument, but got " + std::to_string(argValues.size()) + " instead.");

        return Value(argValues[0].toString());
    }
    else if (funcName == "type") {
        if (argValues.size() != 1) throw std::runtime_error("Argument Error : string() expects 1 argument, but got " + std::to_string(argValues.size()) + " instead.");

        int type = argValues[0].getType();

        switch(type) {
            case Value::NONE:     return Value("null");
            case Value::NUMBER:   return Value("number");
            case Value::STRING:   return Value("string");
            case Value::ARRAY:    return Value("array");
            case Value::TABLE:    return Value("table");
            case Value::FUNCTION: return Value("function");
            default:              return Value("unknown");
        }
    }

    throw std::runtime_error("Unknown built-in function: " + funcName);
}

Value IndexAccessNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    std::string targetGroup = resolvePath(scope, currentGroup);

    if (!env.count(targetGroup) || !env[targetGroup].count(name)) {
        throw std::runtime_error("Array '" + name + "' not found.");
    }

    Value& arrayVal = env[targetGroup][name];

    if (arrayVal.getType() != Value::ARRAY) {
        throw std::runtime_error("Type Error: '" + name + "' is not an array.");
    }

    Value idxVal = index->evaluate(env, currentGroup);
    
    if (idxVal.getType() != Value::NUMBER) {
        throw std::runtime_error("Index must be a number.");
    }

    int i = static_cast<int>(idxVal.asNumber());

    if (i < 0 || i >= static_cast<int>(arrayVal.asList().size())) {
        throw std::runtime_error("Index out of bounds: " + std::to_string(i));
    }

    return arrayVal.asList()[i]; 
}

Value FunctionNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value funcValue(parameters, body);
    const std::string& functionName = "sub@" + this->name;
    env[currentGroup][functionName] = funcValue; 

    return funcValue;
}

Value FunctionCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value funcVal = env["global"]["sub@" + funcName];

    if(funcVal.getType() != Value::FUNCTION) throw std::runtime_error("Type Error : " + funcName + " is not a function.");

    std::vector<Value> evaluatedArgs;

    for (const auto& arg : arguments){
        evaluatedArgs.emplace_back(arg->evaluate(env, currentGroup));
    }

    std::string localScope = "call_" + funcName + "_" + std::to_string(rand()); // for functions local scopes

    auto& params = funcVal.asFunction()->params;

    if(params.size() != evaluatedArgs.size()) throw std::runtime_error("Argument Error : Argument count mismatch on function call " + funcName);
    
    for (size_t i = 0; i < params.size(); ++i) {
        env[localScope][params[i]] = evaluatedArgs[i];
    }

    Value result;
    try {
        for (const auto& bodyNode : funcVal.asFunction()->body){
            result = bodyNode->evaluate(env, localScope);
        }
    } catch (const ReturnException& e) {
        result = e.value; 
    }

    env.erase(localScope);

    return result;
}

Value ReturnNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value val = expression->evaluate(env, currentGroup);
    throw ReturnException{val};
}

Value MethodCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    VariableNode* var = dynamic_cast<VariableNode*>(receiver.get());

    if(var){
        std::string targetGroup = resolvePath(var->getScope(), currentGroup);
        Value& target = env[targetGroup][var->getName()];

        /*
            Array methods are implemented from here
            Current available methods : size, push, pop, delete, sort
        */
       
        if(methodName == "size"){
            if(target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method size() on non-array!");

            return Value(static_cast<double>(target.asList().size()));
        }

        if (methodName == "push"){
            if(target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method push() on non-array!");

            Value val = arguments[0]->evaluate(env, currentGroup);
            target.asList().emplace_back(val);
            return val;
        }

        if(methodName == "pop"){
            if(target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method pop() on non-array!");
            if(target.asList().empty()) throw std::runtime_error("Index Error: pop() from empty array.");
            if(!arguments.empty()) throw std::runtime_error("Argument Error: pop() expects 0 arguments, but got " + std::to_string(arguments.size()) + ".");

            Value lastValue = target.asList().back();
            
            target.asList().pop_back();

            return Value(true);
        }

        if(methodName == "delete"){
            if(target.getType() != Value::ARRAY) throw std::runtime_error("Type Error : Called method delete() on non-array!");
            if(arguments.size() != 1) throw std::runtime_error("Argument Error: delete() expects exactly 1 argument, but got " + std::to_string(arguments.size()) + " instead.");

            Value val = arguments[0]->evaluate(env, currentGroup);

            auto it = std::find(target.asList().begin(), target.asList().end(), val);

            if (it == std::end(target.asList())) throw std::runtime_error("Value error : Could not find given value in array!");

            target.asList().erase(it);

            return Value(true);
        }

        if (methodName == "sort") {
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: sort() called on non-array!");
            if (!arguments.empty()) throw std::runtime_error("Argument Error: sort() expects 0 arguments.");
            
            for(auto& el : target.asList()){
                if(el.getType() != Value::NUMBER) throw std::runtime_error("Value Error: Cannot sort string values!");
            }

            std::sort(target.asList().begin(), target.asList().end());

            return Value(target); 
        }

        if(methodName == "place_all"){
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: place_all() called on non-array!");
            if (arguments.size() > 2) throw std::runtime_error("Argument Error: place_all() expects 2 arguments, but got " + std::to_string(arguments.size()) + " instead.");

            Value element = arguments[0]->evaluate(env, currentGroup);
            Value count = arguments[1]->evaluate(env, currentGroup);

            std::vector<Value> arr;

            for(size_t i = 0; i < count.asNumber(); i++){
                arr.emplace_back(element);
            }

            return Value(arr);
        }

        if(methodName == "reverse"){
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: reverse() called on non-array!");
            if (arguments.size() > 0) throw std::runtime_error("Argument Error: reverse() expects 0 arguments, but got " + std::to_string(arguments.size()) + " instead.");

            std::reverse(target.asList().begin(), target.asList().end());

            return Value(target);
        }
        if(methodName == "clear"){
            if (target.getType() != Value::ARRAY) throw std::runtime_error("Type Error: clear() called on non-array!");
            if (arguments.size() > 0) throw std::runtime_error("Argument Error: clear() expects 0 arguments, but got " + std::to_string(arguments.size()) + " instead.");

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
            
            // optimization idea: if the body returns a "return" flag, stop the loop
            // and pass the return value up the chain.
            // example implementation : if (lastResult.isReturn()) return lastResult;

        } catch (const BreakException& e) {
            break;
        } catch (const ContinueException& e) {
            continue;
        }
    }
    return lastResult; 
}

Value BlockNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value lastValue;
    
    for (const auto& statement : statements) {
        lastValue = statement->evaluate(env, currentGroup);
    }
    
    return lastValue; 
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
};
