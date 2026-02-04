@echo off
setlocal

set CXX=g++

set CXXFLAGS=-std=c++17 -O3 -Wall -Wextra -I"./vendor/glfw/include"
set OUT=vyne.exe

set LDFLAGS=-L"./vendor/glfw/lib-mingw-w64" -lglfw3 -lgdi32 -lopengl32 -luser32 -lshell32

set SRC_FILES=main.cpp ^
vyne/vm/vm.cpp ^
vyne/compiler/codegen/chunk.cpp ^
vyne/compiler/codegen/codegen.cpp ^
vyne/compiler/lexer/lexer.cpp ^
vyne/compiler/parser/parser.cpp ^
vyne/compiler/ast/ast.cpp ^
vyne/compiler/ast/value.cpp ^
vyne/modules/vcore/vcore.cpp ^
vyne/modules/vglib/vglib.cpp ^
vyne/modules/vmem/vmem.cpp ^
cli/repl.cpp ^
cli/file_handler.cpp

echo ---------------------------------------
echo Building Vyne Interpreter (Windows) with GLFW...
echo ---------------------------------------

%CXX% %CXXFLAGS% %SRC_FILES% -o %OUT% %LDFLAGS%

if %ERRORLEVEL% EQU 0 (
    echo Build Successful: %OUT% created.
    
    if "%1"=="--test" (
        if not "%2"=="" (
            echo Running tests/%2.vy with Bytecode...
            .\%OUT% --bytecode tests/%2.vy
        ) else (
            echo Running default bench.vy...
            .\%OUT% --bytecode tests/bench.vy
        )
    )
) else (
    echo.
    echo [ERROR] Build failed. Check the errors above.
    pause
)

endlocal