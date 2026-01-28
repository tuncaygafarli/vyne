@echo off
setlocal

set CXX=g++
set CXXFLAGS=-std=c++17 -g -Wall -Wextra -Wpedantic
set OUT=vyne.exe
set SRC_FILES=main.cpp vyne/compiler/lexer/lexer.cpp vyne/compiler/parser/parser.cpp vyne/compiler/ast/ast.cpp vyne/modules/vcore.cpp

echo ---------------------------------------
echo Building Vyne Interpreter (Windows)...
echo ---------------------------------------

%CXX% %CXXFLAGS% %SRC_FILES% -o %OUT%

if %ERRORLEVEL% EQU 0 (
    echo Build Successful: %OUT% created.
    
    if "%1"=="--test" (
        echo Running module_test.vy...
        .\%OUT% tests/module_test.vy
    )
) else (
    echo Build Failed! Check the errors above.
    exit /b 1
)

endlocal