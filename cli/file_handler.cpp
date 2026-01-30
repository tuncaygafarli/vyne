#include "file_handler.h"

int runFile(const std::string& filename, SymbolContainer& env){
    size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos || filename.substr(dotPos + 1) != "vy" || filename.substr(dotPos + 1) != "vyne") {
        std::cerr << RED << "Error: File must end in .vy ( .vyne )" << RESET << "\n";
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << RED << "Could not open file: " << filename << RESET << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string& content = buffer.str();

    try {
        auto tokens = tokenize(content);
        Parser parser(tokens);
        
        auto programRoot = parser.parseProgram();

        auto start = std::chrono::high_resolution_clock::now();

        programRoot->evaluate(env);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = end - start;
        std::cout << GREEN << "\nExecution finished in: " << ms.count() << "ms" << RESET;
    } catch (const std::exception& e) {
        std::cerr << RED << "Runtime/Compilation Error: " << e.what() << RESET << "\n";
    }

    return 0;
}