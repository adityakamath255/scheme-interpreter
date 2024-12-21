#include "interface.hpp"
#include "primitives.cpp"
#include "syntax.cpp"
#include "parsing.cpp"

using namespace scheme;

environment*
install_initial_environment() {
  auto ret = new environment();
  for (const auto& p : prims) {
    ret->define_variable(p.first, new primitive(p.second));
  }
  ret->define_variable(symbol("#t"), true);
  ret->define_variable(symbol("#f"), false);
  return ret;
}

auto init_env = install_initial_environment();

sc_obj
interpret(const string& code) {
  return eval(classify(*parse(tokenize(code))), init_env);
}

string 
read() {
  string line, input;
  int open_parens = 0;

  while (true) {
    getline(cin, line);
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
      string input_expr = read();
      if (input_expr == "exit\n") 
        return; 
      auto result = interpret(input_expr);
      display(result);
      cout << "\n";
    } 
    catch (const runtime_error& e) {
        cout << "\nERROR: " << e.what() << "\n";
    }
  }
}

int 
main() {
  driver_loop();
}
