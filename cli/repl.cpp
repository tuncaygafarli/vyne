#include "repl.h"

void init_REPL(std::string& input, SymbolContainer& env){
    std::cout << BOLD << CYAN << "Vyne Interpreter v1.0" << RESET << "\n";
    std::cout << "Type " << RED << "exit" << RESET << " to quit.\n\n";

    while (true) {
        std::cout << GREEN << ">> " << RESET;
        if (!std::getline(std::cin, input)) break;

        if (input == "exit") break;
        if (input.empty()) continue;

        if (input == "view tree") {
            std::cout << YELLOW << "--- Current Symbol env ---" << RESET << "\n";
            bool hasAnyVariables = false;

            for (const auto& [groupName, table] : env) {
                for (const auto& [varId, val] : table) {
                    hasAnyVariables = true;
                    
                    std::string realName = StringPool::instance().get(varId);

                    if (groupName == "global") {
                        std::cout << BOLD << realName << RESET << " = ";
                    } else {
                        std::cout << CYAN << groupName << RESET << "." << BOLD << realName << RESET << " = ";
                    }

                    val.print(std::cout);
                    std::cout << "\n";
                }
            }

            if (!hasAnyVariables) {
                std::cout << "(no variables defined)" << "\n";
            }
            std::cout << YELLOW << "-----------------------------" << RESET << "\n";
            continue;
        }

        try {
            auto tokens = tokenize(input);
            Parser parser(tokens);

            auto root = parser.parseProgram();
            if (root) {
                Value result;
                try {
                    result = root->evaluate(env); 
                } 
                catch (const ReturnException& e) {
                    result = e.value; 
                }

                if (result.getType() != Value::NONE) { 
                    result.print(std::cout);
                    std::cout << "\n";
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << RED << "Error: " << e.what() << RESET << "\n";
        }
    }
}