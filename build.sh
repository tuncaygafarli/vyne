#!/bin/bash

g++ -std=c++17 -g main.cpp vyne/compiler/lexer/lexer.cpp vyne/compiler/parser/parser.cpp vyne/compiler/ast/ast.cpp vyne/modules/vcore.cpp -o vyne -Wall -Wextra

if [ $? -ne 0 ]; then
    echo "Build Failed!"
    exit 1
else
    echo "Build Successful: vyne created."
fi
