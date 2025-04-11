#include "common.hpp"
#include "primitives.cpp"
#include "evaluation.cpp"
#include "parsing.cpp"
#include <fstream>

using namespace Scheme;

Environment
install_initial_environment() {
  Environment env;
  for (const auto& p : prims) {
    env.define_variable(p.first, new Primitive(p.second));
  }
  for (const auto& p : consts) {
    env.define_variable(p.first, p.second);
  }
  return Environment(env);
}

auto init_env = install_initial_environment();

Obj
interpret(const string& code) {
  const auto tokens = tokenize(code);
  const auto AST_0 = parse(tokens);
  const auto AST_1 = classify(AST_0); 
  return eval(AST_1, &init_env);
}

template<class INPUT>
string 
read(INPUT& in) {
  string line, input;
  int open_parens = 0;

  while (true) {
    getline(in, line);
    bool is_comment = false;

    for (char& c : line) {
      switch (c) {
        case ';':
          is_comment = true;
          break;
        case '(':
          open_parens++;
          break;
        case ')':
          open_parens--;
          if (open_parens < 0) {
            throw runtime_error("imbalanced parentheses");
          }
          break;
      }

      if (is_comment) {
        break;
      }
      else {
        input += c;
      }
    }

    input += "\n";

    if (open_parens == 0) 
      break;
  }

  return input;
}

void
driver_loop() {
  while (true) {
    try {
      cout << ">>> ";
      string input_expr = read(std::cin);
      if (input_expr == "exit\n") 
        return;
      auto result = interpret(input_expr);
      display(result);
      cout << "\n";
    } 
    catch (const runtime_error& e) {
        cout << "\nERROR: " << e.what() << "\n";
    }
    catch (TailCall tc) {
      auto result = apply(tc.proc, tc.args);
      display(result);
      cout << "\n";
    }
  }
}

void
run_file(const char *filename) {
  std::ifstream in;
  in.open(filename);
  Obj result;
  while (true) {
    try {
      const string& input_expr = read(in);
      if (in.eof()) {
        break;
      }
      interpret(input_expr);
    } 
    catch (const runtime_error& e) {
      cout << "\nERROR: " << e.what() << "\n";
      return; 
    }
    catch (TailCall tc) {
      apply(tc.proc, tc.args);
    }
  }
  cout << "\n";
  driver_loop();
}

int 
main(const int argc, const char **argv) {
  if (argc == 1) {
    driver_loop();
  }
  else if (argc == 2) {
    run_file(argv[1]);
  }
  else {
    cout << "Usage: ./interpreter [file-to-run]" << std::endl;
  }
}
