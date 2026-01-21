#include <iostream>
#include <string>

#include "lexer/lexer.h"
#include "parser/parser.h"

int main(int argc, char* argv[]) {
	std::string expression;
	std::getline(std::cin, expression);

	try {
		auto tokens = tokenize(expression);
		Parser parser(tokens);
		double result = parser.parseExpression();
		std::cout << "Result : " << result << "\n";
	} catch (const std::exception& e) {
		std::cerr << "Math Error: " << e.what() << std::endl;
	}

	return 0;
}