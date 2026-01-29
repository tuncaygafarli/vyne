# 🌿 Vyne

[![Vyne CI](https://github.com/tuncaygafarli/vyne/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/tuncaygafarli/vyne/actions/workflows/c-cpp.yml)

## DISCLAIMER

This repository is under maintanance, meaning that current MD file is being used for personal note purposes.

## Current availabilities

1. Arithmetic operations ( +, -, \*, /, <, >, == )
2. Built-in functions
   - log()
   - sizeof()
   - type()
   - string()
3. Arrays
   - Built-in array methods
     - pop()
     - push()
     - delete()
     - sort()
     - place_all()
     - reverse()
     - clear()
4. Functions
   - Defined by `sub` keyword, taking arguments
5. Groups
   - Defined by `group` keyword, scoping is enabled
6. While loops
7. If statements
8. Modules
   - List of built-in modules
     - vcore

## 📘 Documentation Walkthrough

Vyne's engine architecture is fully documented using **Doxygen**. This allows you to explore the interpreter's internals through a searchable web interface, complete with class diagrams and function call graphs.

### 🛠 Generating the Documentation

To build the documentation locally, ensure you have [Doxygen](https://www.doxygen.nl/) and [Graphviz](https://graphviz.org/) installed, then run:

```bash
doxygen Doxyfile
```

Once the process finishes, open the following file in your browser:
`docs/html/index.html`

### 🔍 Navigating the Engine

The documentation provides several powerful ways to understand how Vyne works:

- **Abstract Syntax Tree (AST) Hierarchy**:
  Navigate to `Classes -> Class Hierarchy`. This visualizes how every language feature (like `WhileNode`, `BinOpNode`, or `FunctionNode`) inherits from the base `ASTNode`.

- **Collaboration Diagrams**:
  Each class page features a diagram showing which other objects it depends on. For example, you can see how an `AssignmentNode` interacts with the `SymbolContainer`.

- **Function Call Graphs**:
  Every `evaluate()` method includes a flowchart showing which sub-functions are called during execution. This is extremely helpful for tracing how the interpreter processes complex Vyne scripts.

- **Native Module Bindings**:
  Explore the `modules` namespace to see the C++ implementation of `vglib` (the donut renderer) and `vcore`. You can view the raw C++ math directly alongside the documentation.

### 🏗 Project Structure

- **`vyne/compiler`**: The Lexer and Parser that turn source code into an AST.
- **`vyne/core`**: The main execution engine and the `Value` system.
- **`vyne/modules`**: Native C++ extensions that provide high-performance features to the language.
