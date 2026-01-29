#!/bin/bash

set -e

CXX=g++
CXXFLAGS="-std=c++17 -g -Wall -Wextra -Wpedantic"
OUT="vyne"
SRC_FILES="main.cpp vyne/compiler/lexer/lexer.cpp vyne/compiler/parser/parser.cpp vyne/compiler/ast/ast.cpp vyne/compiler/ast/value.cpp vyne/modules/vcore/vcore.cpp vyne/modules/vglib/vglib.cpp cli/file_handler.cpp cli/repl.cpp"

echo "---------------------------------------"
echo "Building Vyne Interpreter (Linux/macOS)..."
echo "---------------------------------------"

$CXX $CXXFLAGS $SRC_FILES -o $OUT

echo "Build Successful: $OUT created."

if [ "$1" == "--test" ]; then
    if [ -n "$2" ]; then
        echo "Running tests/${2}_test.vy..."
        ./$OUT "tests/${2}_test.vy"
    else
        echo "Running default stress_test.vy..."
        ./$OUT "tests/stress_test.vy"
    fi
fi