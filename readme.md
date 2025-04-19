# Scheme Interpreter

A lightweight Scheme interpreter implemented in C++23, featuring proper tail-call optimization, first-class procedures, and core Scheme functionality.

## Features

- R5RS-inspired Scheme implementation
- Proper tail-call optimization
- First-class procedures and closures
- Symbol interning for efficient symbol comparison
- Comprehensive set of primitive operations
- REPL interface

## Building

Requirements:
- C++23 compiler (tested with g++)
- Make

To build:

```bash
make
```

This will produce an executable named `scheme`.

## Usage

### REPL Mode

Launch the interpreter in REPL mode:

```bash
./scheme
```

### Run a File

Execute code from a file:

```bash
./scheme path/to/file.scm
```

Run without entering REPL mode after execution:

```bash
./scheme --no-repl path/to/file.scm
```

## Examples

### Basic Arithmetic
```scheme
(+ 1 2 3)
; => 6

(* 2 3 4)
; => 24
```

### Defining Functions
```scheme
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(factorial 5)
; => 120
```

### Creating Lists
```scheme
(define lst (list 1 2 3 4 5))

(map (lambda (x) (* x x)) lst)
; => (1 4 9 16 25)
```

## Supported Forms

- Special forms: `define`, `lambda`, `if`, `begin`, `quote`, `set!`, `let`, `cond`
- Boolean operators: `and`, `or`, `not`
- List manipulation: `cons`, `car`, `cdr`, `list`, `append`, `map`, `filter`
- Arithmetic: `+`, `-`, `*`, `/`, `quotient`, `remainder`, `expt`
- Comparisons: `<`, `>`, `=`, `<=`, `>=`, `eq?`, `equal?`
- Type predicates: `null?`, `boolean?`, `number?`, `symbol?`, `string?`, `pair?`, `procedure?`
- Math functions: `abs`, `sqrt`, `sin`, `cos`, `log`, `max`, `min`, `even?`, `odd?`, `ceil`, `floor`, `round`
- I/O: `display`, `newline`

## Project Structure

- `environment.cpp`: Environment and variable handling
- `evaluation.cpp`: Core evaluation logic
- `expressions.cpp`: Expression type implementations
- `lexer.cpp`: Tokenization of input
- `main.cpp`: Entry point and REPL
- `parser.cpp`: Parser for Scheme syntax
- `primitives.cpp`: Built-in primitive functions
- `stringify.cpp`: Converting Scheme objects to strings
- `tco.cpp`: Tail call optimization
- `types.cpp`: Core type definitions and operations

## Roadmap

- Add garbage collection (mark and sweep)
- Implement hygienic macros
- Add proper continuations
- Build a standard library
- Add proper module system
- Implement file I/O operations
- Create a comprehensive test suite
- Improve error handling and diagnostics

## License

This project is licensed under the MIT License - see the LICENSE file for details.
