#include "vm.h"
#include <iostream>

InterpretResult VM::interpret(Chunk& c) {
    this->chunk = &c;
    this->ip = chunk->code.data();
    return run();
}

InterpretResult VM::run() {
    #define READ_BYTE() (*ip++)
    #define READ_CONSTANT() (chunk->constants[READ_BYTE()])

    for (;;) {
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant); 
                break;
            }
            case OP_ADD: {
                Value b = pop();
                Value a = pop();

                double result = std::get<double>(a.data) + std::get<double>(b.data);
                push(Value(result));
                break;
            }
            case OP_SUBTRACT : {
                Value b = pop();
                Value a = pop();

                double result = std::get<double>(a.data) - std::get<double>(b.data);
                push(Value(result));
                break;
            }
            case OP_MULTIPLY : {
                Value b = pop();
                Value a = pop();

                double result = std::get<double>(a.data) * std::get<double>(b.data);
                push(Value(result));
                break;
            }
            case OP_DIVIDE : {
                Value b = pop();
                Value a = pop();

                if (std::get<double>(b.data) == 0) {
                    std::cerr << "Runtime Error: Division by zero." << std::endl;
                    return INTERPRET_RUNTIME_ERROR;
                }

                double result = std::get<double>(a.data) / std::get<double>(b.data);
                push(Value(result));
                break;
            }

            case OP_DEFINE_GLOBAL : {
                Value nameValue = READ_CONSTANT();
                std::string name = *std::get<std::shared_ptr<std::string>>(nameValue.data);

                Value val = pop();

                this->globals[this->currentGroup][StringPool::intern(name)] = val;

                break;
            }
            case OP_GET_GLOBAL: {
                Value nameValue = READ_CONSTANT();
                std::string name = *std::get<std::shared_ptr<std::string>>(nameValue.data);

                uint32_t nameId = StringPool::intern(name);

                auto& table = globals[currentGroup];
                auto it = table.find(nameId);

                if (it != table.end()) {
                    push(it->second);
                } else {
                    std::cerr << "Runtime Error: Undefined variable '" << name << "'.\n";
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_RETURN: {
                Value finalResult = pop();
                std::cout << "Result: ";
                finalResult.print(std::cout);
                std::cout << std::endl;
                return INTERPRET_OK;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
}