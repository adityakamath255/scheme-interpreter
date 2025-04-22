#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "evaluation.hpp"
#include "primitives.hpp"
#include "stringify.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

using namespace Scheme;

Environment *
install_initial_environment() {
  auto env = new Environment();
  for (const auto& p : get_primitive_functions()) {
    env->define_variable(p.first, new Primitive(p.second));
  }
  for (const auto& p : get_consts()) {
    env->define_variable(p.first, p.second);
  }
  return new Environment(env);
}

Obj
interpret(const std::string& code, Environment *env) {
  const auto tokens = Lexer(code).tokenize();
  const auto AST_0 = Parser(tokens).parse();
  auto AST_1 = classify(AST_0); 
  return as_obj(eval(AST_1, env));
}

template<class INPUT>
std::string 
read(INPUT& in) {
  std::ostringstream result;
  std::string line;
  int open_parens = 0;

  while (std::getline(in, line)) {
    size_t comment_pos = line.find(';');
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }

    for (char c : line) {
      switch (c) {
        case '(':
          open_parens++;
          break;
        case ')':
          open_parens--;
          if (open_parens < 0) {
            throw std::runtime_error("imbalanced parentheses");
          }
          break;
      }
    }

    result << line << '\n';

    if (open_parens == 0) 
      break;
  }

  if (open_parens != 0) {
    throw std::runtime_error("unmatched parentheses at end of input");
  }

  return result.str();
}

void
driver_loop(Environment *env = nullptr) {
  if (env == nullptr) {
    env = install_initial_environment();
  }
  while (true) {
    try {
      std::cout << ">>> ";
      std::string input_expr = read(std::cin);
      if (input_expr == "exit\n") 
        return;
      auto result = interpret(input_expr, env);
      if (!is_void(result)) {
        std::cout << stringify(result);
        std::cout << "\n";
      }
    } 
    catch (std::runtime_error& e) {
      std::cerr << "ERROR: " << e.what() << "\n";
    }
    catch (std::bad_variant_access e) {
      std::cerr << "ERROR: incorrect type\n";
    }
  }
}

void
run_file(const char *filename, bool enter_driver_loop) {
  std::ifstream in;
  in.open(filename);
  auto env = install_initial_environment();
  while (true) {
    try {
      const std::string& input_expr = read(in);
      if (in.eof()) {
        break;
      }
      interpret(input_expr, env);
    } 
    catch (std::runtime_error& e) {
      std::cerr << "\nERROR: " << e.what() << "\n";
      return; 
    }
  }
  std::cout << "\n";
  if (enter_driver_loop) {
    driver_loop(env);
  }
}

int 
main(const int argc, const char **argv) {
  if (argc == 1) {
    driver_loop();
  }
  else if (argc == 2) {
    run_file(argv[1], true);
  }
  else if (argc == 3 && std::string(argv[1]) == "--no-repl") {
    run_file(argv[2], false);
  }
  else {
    std::cout << "Usage: ./scheme [--no-repl] [file-to-run]" << std::endl;
  }
}
