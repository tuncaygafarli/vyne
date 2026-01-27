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

    if (l.type == Value::STRING && r.type == Value::STRING) {
        if (op == '+') return Value(l.text + r.text);
        throw std::runtime_error("Type Error: Only '+' allowed for strings.");
    }

    if (l.type == Value::ARRAY && r.type == Value::ARRAY) {
        if (op == '+') {
            Value result = l;

            result.list.insert(result.list.end(), r.list.begin(), r.list.end());
            return result;
        }
        throw std::runtime_error("Type Error: Only '+' allowed for strings, numbers and arrays.");
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

    return Value(); 
}

Value SizeofNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value val = expression->evaluate(env, currentGroup);
    
    return Value(static_cast<double>(val.getBytes()));
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

Value FunctionNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value funcValue(parameters, body);
    const std::string& functionName = "sub@" + this->name;
    env[currentGroup][functionName] = funcValue; 

    return funcValue;
}

Value FunctionCallNode::evaluate(SymbolContainer& env, std::string currentGroup) const {
    Value funcVal = env["global"]["sub@" + funcName];

    if(funcVal.type != Value::FUNCTION) throw std::runtime_error("Type Error : " + funcName + " is not a function.");

    std::vector<Value> evaluatedArgs;

    for (const auto& arg : arguments){
        evaluatedArgs.emplace_back(arg->evaluate(env, currentGroup));
    }

    std::string localScope = "call_" + funcName + "_" + std::to_string(rand()); // for functions local scopes

    auto& params = funcVal.function->params;

    if(params.size() != evaluatedArgs.size()) throw std::runtime_error("Argument Error : Argument count mismatch on function call " + funcName);
    
    for (size_t i = 0; i < params.size(); ++i) {
        env[localScope][params[i]] = evaluatedArgs[i];
    }

    Value result;
    try {
        for (const auto& bodyNode : funcVal.function->body){
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
            if(target.type != Value::ARRAY) throw std::runtime_error("Type Error : Called method size() on non-array!");

            return Value(static_cast<double>(target.list.size()));
        }

        if (methodName == "push"){
            if(target.type != Value::ARRAY) throw std::runtime_error("Type Error : Called method push() on non-array!");

            Value val = arguments[0]->evaluate(env, currentGroup);
            target.list.emplace_back(val);
            return val;
        }

        if(methodName == "pop"){
            if(target.type != Value::ARRAY) throw std::runtime_error("Type Error : Called method pop() on non-array!");
            if(target.list.empty()) throw std::runtime_error("Index Error: pop() from empty array.");
            if(!arguments.empty()) throw std::runtime_error("Argument Error: pop() expects 0 arguments, but got " + std::to_string(arguments.size()) + ".");

            Value lastValue = target.list.back();
            
            target.list.pop_back();

            return Value(true);
        }

        if(methodName == "delete"){
            if(target.type != Value::ARRAY) throw std::runtime_error("Type Error : Called method delete() on non-array!");
            if(arguments.size() != 1) throw std::runtime_error("Argument Error: delete() expects exactly 1 argument, but got " + std::to_string(arguments.size()) + " instead.");

            Value val = arguments[0]->evaluate(env, currentGroup);

            auto it = std::find(target.list.begin(), target.list.end(), val);

            if (it == std::end(target.list)) throw std::runtime_error("Value error : Could not find given value in array!");

            target.list.erase(it);

            return Value(true);
        }

        if (methodName == "sort") {
            if (target.type != Value::ARRAY) throw std::runtime_error("Type Error: sort() called on non-array!");
            if (!arguments.empty()) throw std::runtime_error("Argument Error: sort() expects 0 arguments.");
            
            for(auto& el : target.list){
                if(el.type != Value::NUMBER) throw std::runtime_error("Value Error: Cannot sort string values!");
            }

            std::sort(target.list.begin(), target.list.end());

            return Value(true); 
        }

        if(methodName == "place_all"){
            if (target.type != Value::ARRAY) throw std::runtime_error("Type Error: place_all() called on non-array!");
            if (arguments.size() > 2) throw std::runtime_error("Argument Error: place_all() expects 2 arguments, but got " + std::to_string(arguments.size()) + " instead.");

            Value element = arguments[0]->evaluate(env, currentGroup);
            Value count = arguments[1]->evaluate(env, currentGroup);

            std::vector<Value> arr;

            for(size_t i = 0; i < count.number; i++){
                arr.emplace_back(element);
            }

            // returns a Value list
            return Value(arr);
        }
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
