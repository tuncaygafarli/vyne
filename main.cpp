#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"

int main(int argc, char* argv[]) {
	SymbolForest forest;
	forest["default"] = {};
	
	forest["default"]["pi"] = 3.14159;
	forest["default"]["x"] = 10.0;

	std::string input;
	std::cout << "TF-Compiler Shell (Type 'exit' to quit)" << "\n";

	if (argc > 1) {
		std::string filename = argv[1];

		size_t dotPos = filename.find_last_of(".");
		if (dotPos == std::string::npos || filename.substr(dotPos + 1) != "tf") {
			std::cerr << "Error: File must end in .tf" << "\n";
			return 1;
		}

		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Could not open file: " << filename << "\n";
			return 1;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();

		auto tokens = tokenize(content);

		Parser parser(tokens);

		try {
			while (parser.peekToken().type != TokenType::End) {
				auto ast = parser.parseStatement();
				if (ast) {
					ast->evaluate(forest);
				}
			}
		} catch (const std::exception& e) {
			std::cerr << "Runtime Error: " << e.what() << "\n";
		}

		return 0;
	}
		while (true) {
			std::cout << ">> ";
			std::getline(std::cin, input);

			if (input == "exit") break;
			if (input.empty()) continue;

			// debug commands
			if (input == "view tree") {
				std::cout << "--- Current Symbol Forest ---" << "\n";

				bool hasAnyVariables = false;

				for (const auto& [groupName, table] : forest) {

					for (const auto& [varName, val] : table) {
						hasAnyVariables = true;

						if (groupName == "default") {
							std::cout << varName << " = ";
						}
						else {
							std::cout << groupName << "[" << varName << "] = ";
						}

						if (val.isString) {
							std::cout << "\"" << val.text << "\"" << "\n";
						}
						else {
							std::cout << val.number << "\n";
						}
					}
				}

				if (!hasAnyVariables) {
					std::cout << "(no variables defined)" << "\n";
				}

				std::cout << "-----------------------------" << "\n";
				continue;
			}

			try {
				auto tokens = tokenize(input);
				Parser parser(tokens);

				auto root = parser.parseStatement();
				if (root) {
					Value result = root->evaluate(forest);
					result.print();
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << "\n";
			}
	}
	return 0;
}