#pragma once

#include <thread>
#include <chrono>

#include "../compiler/ast/ast.h"
#include "../compiler/ast/value.h"

void setupBuiltIns(SymbolContainer& env);