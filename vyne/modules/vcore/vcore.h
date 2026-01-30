#pragma once
#include <ctime>
#include <chrono>
#include <thread>
#include <random>
#include <iostream>
#include <filesystem>

#include "../../compiler/ast/ast.h"
#include "../../compiler/ast/value.h"

void setupVCore(SymbolContainer& env, StringPool& pool);