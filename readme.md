# Scheme Interpreter (C++23)

A minimal, fast, and correct Scheme interpreter written in C++23. 

## Features

- Basic types: Booleans, numbers, strings, symbols, pairs, procedures, void.

- Special forms: lambda, define, if, cond, let, quote, set!, begin, and, or

- Proper tail recursion

- Lexical closures

- 50+ built-in primitives:

  - Math: +, -, *, /, sqrt, log, expt

  - Lists: map, filter, car, cdr, cons, append

  - Comparisons: =, <, >, eq?, equal?

  - Type checks: number?, pair?, procedure?, null?

  - I/O: display, newline, error

## Known Limitations

- No proper memory management or garbage collection yet.

- No continuations or macros.

- Error messages can be vague.

- Floats only - no exact rationals.

## Build

### Requirements

- C++23 compatible compiler

- GNU Make

### Compile & Run

```bash 
git clone https://github.com/adityakamath255/scheme-interpreter 
cd scheme-interpreter 

make # builds 'scheme' 
./scheme # starts REPL 
./scheme file.scm # runs a Scheme script 
./scheme --no-repl file.scm # runs a Scheme script without invoking a REPL loop

make clean # removes build artifacts 
```
