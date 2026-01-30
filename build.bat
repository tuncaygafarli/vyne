@echo off
setlocal

set CXX=g++
set CXXFLAGS=-std=c++17 -g -Wall -Wextra -Wpedantic
set OUT=vyne.exe
set SRC_FILES=main.cpp vyne/vm/vm.cpp vyne/compiler/codegen/chunk.cpp vyne/compiler/codegen/codegen.cpp vyne/compiler/lexer/lexer.cpp vyne/compiler/parser/parser.cpp vyne/compiler/ast/ast.cpp vyne/compiler/ast/value.cpp vyne/modules/vcore/vcore.cpp vyne/modules/vglib/vglib.cpp vyne/modules/vmem/vmem.cpp cli/repl.cpp cli/file_handler.cpp

echo ---------------------------------------
echo Building Vyne Interpreter (Windows)...
echo ---------------------------------------

%CXX% %CXXFLAGS% %SRC_FILES% -o %OUT%

if %ERRORLEVEL% EQU 0 (
    echo Build Successful: %OUT% created.
    
    if "%1"=="--test" (
        if not "%2"=="" (
            echo Running tests/%2_test.vy...
            .\%OUT% tests/%2_test.vy
        ) else (
            echo Running default stress_test.vy...
            .\%OUT% tests/stress_test.vy
        )
    )
)

endlocal