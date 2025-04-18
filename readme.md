# Scheme Interpreter (C++23)

A minimal, fast, and correct Scheme interpreter written in C++23. Implements core R7RS semantics with lexical scoping, first-class procedures, and tail-call optimization. 

## Features

- Special forms: lambda, define, if, cond, let, quote, set!, begin

- Proper tail recursion

- Lexical closures

- 50+ built-in primitives:

  - Math: +, -, *, /, sqrt, log, expt

  - Lists: map, filter, car, cdr, cons, append

  - Comparisons: =, <, >, eq?, equal?

  - Type checks: number?, pair?, procedure?, null?

  - I/O: display, newline, error

## Memory Management

Manual. RAII for scoped cleanup. GC planned.

## Build

### Requirements

- C++23-compatible compiler (tested with g++ 13+)

- GNU Make

### Compile & Run

```bash 
git clone https://github.com/adityakamath255/scheme-interpreter 
cd scheme-interpreter 

make # builds 'scheme' 
./scheme # starts REPL 
./scheme file.scm # runs a Scheme script 

make clean # removes build artifacts 
```
