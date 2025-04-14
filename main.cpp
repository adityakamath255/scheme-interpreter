#include "common.hpp"
#include "primitives.hpp"
#include "expressions.hpp"
#include "parsing.hpp"
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
interpret(const string& code, Environment *env) {
  const auto tokens = tokenize(code);
  const auto AST_0 = parse(tokens);
  const auto AST_1 = classify(AST_0); 
  return eval(AST_1, env);
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
driver_loop(Environment *env = nullptr) {
  if (env == nullptr) {
    env = install_initial_environment();
  }
  while (true) {
    try {
      cout << ">>> ";
      string input_expr = read(cin);
      if (input_expr == "exit\n") 
        return;
      auto result = interpret(input_expr, env);
      if (!holds_alternative<Void>(result)) {
        cout << stringify(result);
        cout << "\n";
      }
    } 
    catch (std::runtime_error& e) {
      cerr << "ERROR: " << e.what() << "\n";
    }
    catch (std::bad_variant_access e) {
      cerr << "ERROR: incorrect type\n";
    }
    catch (TailCall tc) {
      auto result = apply(tc.proc, tc.args);
      if (!holds_alternative<Void>(result)) {
        cout << stringify(result);
        cout << "\n";
      }
    }
  }
}

void
run_file(const char *filename) {
  std::ifstream in;
  in.open(filename);
  Obj result;
  auto env = install_initial_environment();
  while (true) {
    try {
      const string& input_expr = read(in);
      if (in.eof()) {
        break;
      }
      interpret(input_expr, env);
    } 
    catch (runtime_error& e) {
      cerr << "\nERROR: " << e.what() << "\n";
      return; 
    }
    catch (TailCall tc) {
      apply(tc.proc, tc.args);
    }
  }
  cout << "\n";
  driver_loop(env);
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
