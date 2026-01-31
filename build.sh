#!/bin/bash

set -e

CXX=g++
CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra -Wpedantic"
OUT="vyne"

SRC_FILES="main.cpp \
vyne/vm/vm.cpp \
vyne/compiler/compiler.cpp \
vyne/compiler/codegen/chunk.cpp \
vyne/compiler/codegen/codegen.cpp \
vyne/compiler/lexer/lexer.cpp \
vyne/compiler/parser/parser.cpp \
vyne/compiler/ast/ast.cpp \
vyne/compiler/ast/value.cpp \
vyne/modules/vcore/vcore.cpp \
vyne/modules/vglib/vglib.cpp \
vyne/modules/vmem/vmem.cpp \
cli/file_handler.cpp \
cli/repl.cpp"

echo "---------------------------------------"
echo "Building Vyne Interpreter (Linux/macOS)..."
echo "---------------------------------------"

$CXX $CXXFLAGS $SRC_FILES -o $OUT

echo "Build Successful: $OUT created."

if [ "$1" == "--test" ]; then
    if [ -n "$2" ]; then
        echo "Running tests/$2.vy with Bytecode..."
        ./$OUT --bytecode "tests/$2.vy"
    else
        echo "Running default bench.vy..."
        ./$OUT --bytecode "tests/bench.vy"
    fi
fi