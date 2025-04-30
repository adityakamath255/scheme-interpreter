# Scheme Interpreter (C++23)

A minimal yet robust Scheme interpreter written from scratch in C++23. This project aims for correctness, clarity, and faithfulness to Scheme's semantics without external dependencies or heavyweight abstractions.

## Features

- **Lexical scoping with closures**
- **Proper tail call optimization** (using trampolining, with constant-space tail recursion)
- **Manual mark-and-sweep garbage collector** (non-moving, pointer-stable)
- **Efficient symbol interning**
- **AST-based evaluation** 
- **Profiling instrumentation** for every evaluation phase
- **50+ built-in primitives**, including:
  - Arithmetic: `+`, `-`, `*`, `/`, `sqrt`, `log`, `expt`, etc.
  - Lists: `cons`, `car`, `cdr`, `append`, `map`, `filter`, `length`, etc.
  - Comparisons: `=`, `<`, `>`, `<=`, `>=`, `eq?`, `equal?`
  - Type checks: `number?`, `pair?`, `vector?`, `procedure?`, `null?`, etc.
  - Vectors: `make-vector`, `vector-ref`, `vector-set!`, `vector-length`
  - I/O: `display`, `newline`, `error`

## Architecture

- **Lexer:** Minimal tokenizer based on string views. Fast but currently limited (no support for escaped strings or quasiquote syntax).
- **Parser:** Recursive descent parser with support for vectors, dotted pairs, quoted expressions.
- **AST Nodes:** Represented as heap-allocated `Expression` subclasses with support for `TailCall` trampolining.
- **Evaluator:** Iterative core that avoids call stack growth during tail-recursive execution.
- **Garbage Collector:** Mark-and-sweep collector manually invoked after each top-level evaluation.
- **Profiler:** Microsecond-level timing instrumentation for lexing, parsing, AST building, evaluation, and garbage collection.

## Limitations

- **No macros** (`syntax-rules` or `define-syntax`) yet.
- **No continuation support** (`call/cc`, etc.).
- **No REPL history or line editing**.
- **Only floating-point numbers** (no exact integers or rationals).
- **Limited reader** (no support for quasiquote, unquote, or custom dispatch).

## Build Instructions

### Requirements

- C++23 compatible compiler (e.g., GCC 13+, Clang 16+)
- GNU Make

### Build

```bash
git clone https://github.com/adityakamath255/scheme-interpreter
cd scheme-interpreter
make       # builds the 'scheme' binary
```

### Run

```bash
./scheme                 # starts REPL
./scheme file.scm       # runs a Scheme script and enters REPL
./scheme --no-repl file.scm  # runs a Scheme script without starting REPL
```

### Clean

```bash
make clean
```

## Example

```scheme
(define (fact n)
  (if (= n 0)
      1
      (* n (fact (- n 1)))))

(fact 10)
```

## Performance

With profiling enabled, the interpreter prints microsecond-level timings for each evaluation phase after every top-level input or script execution.

Example output:

```
Profile:
Lexing:             30 μs
Parsing:            55 μs
AST Building:       120 μs
Evaluating:         200 μs
Garbage Collecting: 40 μs
```

## Status

This interpreter is stable, efficient, and capable of handling a large subset of R5RS-compliant programs. It is designed to be extended in the future with macros, richer numeric types, continuations, and a more powerful reader.

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE) for details.
