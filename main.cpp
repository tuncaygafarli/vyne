#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"

int main(int argc, char* argv[]) {
	SymbolTable globals;
	
	globals["pi"] = 3.14159;
	globals["x"] = 10.0;

	std::string input;
	std::cout << "TF-Compiler Shell (Type 'exit' to quit)" << std::endl;

	if (argc > 1) {
		std::string filename = argv[1];

		size_t dotPos = filename.find_last_of(".");
		if (dotPos == std::string::npos || filename.substr(dotPos + 1) != "tf") {
			std::cerr << "Error: File must end in .tf" << std::endl;
			return 1;
		}

		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Could not open file: " << filename << std::endl;
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
					ast->evaluate(globals);
				}
			}
		} catch (const std::exception& e) {
			std::cerr << "Runtime Error: " << e.what() << std::endl;
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
				std::cout << "--- Current Variables ---" << std::endl;
				if (globals.empty()) {
					std::cout << "(no variables defined)" << std::endl;
				}
				else {
					for (const auto& [name, value] : globals) {
						std::cout << name << " = ";

						if (value.isString) {
							std::cout << value.text << std::endl;
						}
						else {
							std::cout << value.number << std::endl;
						}
					}
				}
				std::cout << "-------------------------" << std::endl;
			}

			try {
				auto tokens = tokenize(input);
				Parser parser(tokens);

				auto root = parser.parseStatement();
				if (root) {
					Value result = root->evaluate(globals);
					std::cout << result.number << std::endl;
				}
			}
			catch (const std::exception& e) {
				std::cerr << "Error: " << e.what() << std::endl;
			}
	}
	return 0;
}