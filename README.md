# Noveris Programming Language

A modern, concise programming language designed for simplicity and efficiency.

## Overview

Noveris is a dynamically-typed programming language with a clean, minimal syntax that focuses on readability and ease of use. The language features function definitions, conditional logic, mathematical operations, and robust control flow.

## Syntax Features

### Variable Assignment
```noveris
set x 10
set y 20
set name "Noveris"
```

### Functions
- Functions are defined with `name(parameters)`
- Functions return values using `res value`
- Functions can be called with `run functionName()`
- Functions support early exit with `out`
- Functions can terminate program with `stop`

### Control Flow
- `if condition:` for conditional execution
- `while condition:` loops; body uses the same indentation/block rules as `if` (see **Loops** below)
- `out` to exit/break from current function
- `res` to return a value and exit function
- `stop` to terminate entire program
- Nested conditions are supported
- Inline if expressions for concise conditional logic

### Loops

**While**

```noveris
set i 0
while i < 3:
    print i
    do i i + 1
```

The condition is re-evaluated each iteration. `out`, `res`, and `stop` inside the body behave like they do in `if` branches.

The body ends at the same places as an `if` **then** branch: before `else`, before a closing `)` that ends an enclosing function, or before a line that starts a new function definition `name(` at the same nesting level. Use a helper `name(` … `)` function if you need a short loop followed by more top-level code.

**For (numeric, step 1)**

```noveris
for n 1 4:
    print n
```

Form: `for` *variable* *startExpr* *endExpr* `:` then one or more statements. The variable is set to *start*; each iteration runs while the current value is **less than or equal to** *end* (re-evaluated every iteration); after the body, the variable increases by `1.0`. *start* and *end* must be numeric (numbers or booleans coerced as in arithmetic).

### Variables
- Variables are assigned with `set variableName value` or `do variableName value`
- Variables can hold numbers, strings, and boolean values
- Variables are dynamically typed
- Function scope is isolated from global scope

### Mathematical Operations
- Basic arithmetic: `+`, `-`, `*`, `/`
- Assignment operations: `do` and `set`
- Parentheses for operation precedence
- Comparison operators: `=`, `>`, `<`, `>=`, `<=`, `!=`
- Logical operators: `&&`, `||`, `!` (`&&` and `||` short-circuit: the right-hand side is skipped when the result is already fixed)

### Comparison Operations
```noveris
if x > 5:
    print "x is greater than 5"

if y < 10:
    print "y is less than 10"

if value = 42:
    print "value equals 42"
```

### Chained comparisons

You can chain comparison operators (same idea as Python). For example, `a <= b <= c` is equivalent to `(a <= b) && (b <= c)`: each pair is compared, and all of them must hold.

If you split the logic across variables, keep the **middle value** in the second check. Storing only `a <= b` gives a boolean, not `b`, so `(a <= b) && (b <= c)` matches the chain; `(a <= b) <= c` does not.

### Data Types
- Numbers (double precision floating point)
- Strings (double-quoted)
- Booleans (`true`/`false`)
- Dynamic typing with automatic conversion

## Example Program

```noveris
set x 10
set y 20
set name "Noveris"

hello(
    print "Hello"
    res true
)

math1(
    if x = 10:
        do x y + 20
        if x = 90:
            print x
            out
        out
    res true
)

if name = "Noveris":
    run math1()
    set x 10
    
    if hello() = true:
        print "hello is true"
    else:
        print "hello is false"
    
    stop
```

## Key Characteristics

- **Minimal Syntax**: Clean, readable syntax with minimal punctuation
- **Dynamic Typing**: No need to declare variable types
- **Function-First**: Functions are first-class citizens
- **Inline Logic**: Supports inline conditional expressions
- **Robust Control Flow**: Multiple exit strategies (out, res, stop)
- **Type Safety**: Automatic type conversion with runtime checks
- **Chained comparisons**: Expressions like `x < y <= z` expand to conjunctions of pairwise comparisons
- **Short-circuit logic**: `&&` and `||` skip evaluating the right operand when it cannot change the result
- **Loops**: `while` and numeric `for` with inclusive end and step 1

## Project Structure

```
Noveris/
├── src/                   # Source implementation files
│   ├── Lexer.cpp          # Lexical analysis (tokenization)
│   ├── Parser.cpp         # Syntax analysis (AST generation)
│   ├── Interpreter.cpp    # Runtime execution
│   └── main.cpp           # Entry point
├── include/               # Header files
│   ├── AST.h              # Abstract Syntax Tree definitions
│   ├── Lexer.h            # Token definitions and lexer interface
│   ├── Parser.h           # Parser interface and method declarations
│   └── Interpreter.h      # Interpreter interface and visitor pattern
├── examples/              # Example programs
│   ├── hello.nv           # Basic "Hello World"
│   ├── math.nv            # Mathematical operations
│   ├── conditions.nv      # Conditional logic
│   ├── functions.nv       # Function definitions
│   ├── boolean.nv         # Boolean operations
│   ├── loops.nv           # while and for loops
│   └── edge_test_*.nv     # Edge case test programs
├── local/                 # Optional local scripts and notes (not required to build)
├── build/                 # Build output directory (created by CMake)
│   └── bin/               # Compiled executable (e.g. `noveris` / `noveris.exe`)
└── README.md              # This file
```

## Building the Project

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)

### Build Instructions

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Generate build files:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build .
```

On Windows with Visual Studio:
```bash
cmake --build . --config Release
```

The executable will be created in `build/bin/` directory.

## Usage

### Running Noveris Programs

To run a Noveris program:
```bash
./bin/noveris path/to/your/file.nv
```

For verbose output (showing lexical analysis, parsing, and execution details):
```bash
./bin/noveris -v path/to/your/file.nv
./bin/noveris --verbose path/to/your/file.nv
```

### String Escapes in Print Statements

The `print` statement supports the following escape sequences:

- `\n` - Newline character
- `\t` - Tab character  
- `\r` - Carriage return
- `\\` - Backslash
- `\"` - Double quote
- `\'` - Single quote

#### Example Usage:
```noveris
print "Hello\\nWorld"          // Outputs: Hello (newline) World
print "Tab\\tSeparated"        // Outputs: Tab (tab) Separated
print "Quote: \\\"Hello\\\""   // Outputs: Quote: "Hello"
print "Path: C:\\\\Users"      // Outputs: Path: C:\Users
```

On Windows (typical CMake single-config build, from the project root):
```powershell
.\build\bin\noveris.exe path\to\your\file.nv
.\build\bin\noveris.exe -v path\to\your\file.nv
```

With a multi-config generator (Visual Studio), the executable may be under `build\bin\Release\` or `build\bin\Debug\` instead.

## Implementation Details

The Noveris interpreter consists of three main components:

1. **Lexer**: Converts source code into tokens using regex-based tokenization
2. **Parser**: Builds an Abstract Syntax Tree (AST) from tokens using recursive descent
3. **Interpreter**: Executes the AST using a visitor pattern

### Language Features

The interpreter supports:
- **Dynamic typing** with numbers, strings, and booleans
- **Function definitions and calls** with proper scope isolation
- **Conditional statements** (if-else) with nested support
- **Mathematical operations** with proper operator precedence
- **Variable assignment and manipulation** with type safety
- **Control flow statements** (out, res, stop) for program control
- **Inline expressions** for concise conditional logic
- **String operations** including concatenation with automatic type conversion
- **Chained comparisons** and **short-circuit** `&&` / `||` as described above
- **`while` and `for` loops** as described in **Loops**

### Error Handling

The language provides clear error messages for:
- **Parse errors** with line and column information (where the parser got stuck)
- **Runtime errors** (undefined variable or function, division by zero, invalid numeric conversion, unknown operator) with **line and column** when the failing expression or statement was given a source location in the AST
- Division by zero protection on `/`

## License

This project is licensed under the MIT License. See the LICENSE file for details.

```
MIT License

Copyright (c) 2026 Noveris

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Contributing

The Noveris language is designed to be simple yet extensible. Key areas for enhancement:
- Additional mathematical operations
- Richer loops (`break`/`continue`, custom step in `for`)
- Array/list data structures
- File I/O operations
- Module system