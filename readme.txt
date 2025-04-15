This is a simple Scheme interpreter written in C++. It parses and evaluates Scheme code, supporting basic operations like arithmetic, conditionals, lambdas, recursion, etc. It’s built with modularity in mind, making it easy to extend and modify.

---

## Files

common.hpp
This file defines the core interface and data structures for the interpreter, including the sc_obj (Scheme objects) and various classes that represent expressions, symbols, and the environment.

stringify.cpp
This file defines the textual representation of objects.

primitives.cpp
This file contains a collection of predefined primitive functions, such as arithmetic operators and basic Scheme functions. The primitives are added to the environment when the interpreter is initialized.

expressions.cpp
This file defines the structures and functions related to the syntax of the language, such as expressions, special forms (e.g., if, lambda, define), and the logic to classify and interpret different types of expressions.

evaluation.cpp
This file defines how expressions get executed.

tco.cpp
This file defines how tail-call optimization is implemented.

parsing.cpp
Here, the tokenizer and recursive-descent parser are implemented. The tokenizer breaks the input code into tokens, and the parser converts those tokens into abstract syntax trees (ASTs) that can be evaluated by the interpreter.

main.cpp
This is the main entry point of the program. It handles user input, runs the interpreter, and interacts with the environment to evaluate and display the results of Scheme code.

--- 

## How to Use

Compile the program:

make

Run the interpreter:

./scheme [--no-repl] [filename] 

The Scheme interpreter can either be run on a specific file, or not. Use the --no-repl file to exit the process after interpretation of the file, rather than beginning the REPL loop.

The interpreter provides an interactive REPL (Read-Eval-Print Loop) for you to interact with the interpreter by typing Scheme expressions directly. 

After compiling and running the program, you will be presented with a prompt like this:

>>>

You can now start typing Scheme expressions at the prompt. For example:

>>> (+ 1 2)
3

The interpreter will evaluate the expression and print the result.

Scheme is known for its heavy use of parentheses. The interpreter will allow you to input multiple lines of code, handling open and close parentheses across lines. The input will continue until the parentheses are balanced. For example:

>>> (define (factorial n)
      (if (= n 0)
          1
          (* n (factorial (- n 1)))))
>>> (factorial 5)
120

The interpreter will ensure that the parentheses are properly balanced before evaluating the expression.

If there is an error, such as mismatched parentheses or invalid syntax, the interpreter will display an error message. For example:

>>> (+ 1 2))
ERROR: imbalanced parentheses

This ensures that you can catch errors as they happen and quickly fix them.

Indentation and line breaks do not affect how the Scheme code is parsed. The interpreter recognizes Scheme code by its syntax, so feel free to format your code for readability.

Any line starting with a semicolon (;) is treated as a comment and will be ignored by the interpreter. For example:

>>> ; This is a comment
>>> (+ 2 3)
5
