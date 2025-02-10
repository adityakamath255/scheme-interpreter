#include "common.hpp"
#include "primitives.cpp"
#include "syntax.cpp"
#include "parsing.cpp"
#include <fstream>

using namespace scheme;

shared_ptr<environment>
install_initial_environment() {
  auto ret = make_shared<environment>();
  for (const auto& p : prims) {
    ret->define_variable(p.first, make_shared<primitive>(p.second));
  }
  ret->define_variable(symbol("#t"), true);
  ret->define_variable(symbol("#f"), false);
  ret->define_variable(symbol("nil"), nullptr);
  auto ret2 = make_shared<environment>(ret);
  return ret2;
}

auto init_env = install_initial_environment();

sc_obj
interpret(const string& code) {
  auto classified = classify(parse(tokenize(code)));
  return eval(classified, init_env);
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

void driver_loop() {
  while (true) {
    try {
      cout << ">>> ";
      string input_expr = read(cin);
      if (input_expr == "exit\n") 
        return ; 
      auto result = interpret(input_expr);
      display(result);
      cout << "\n";
    } 
    catch (const runtime_error& e) {
        cout << "\nERROR: " << e.what() << "\n";
    }
  }
}

void
run_file(const char *filename) {
  ifstream in;
  in.open(filename);
  sc_obj result;
  while (true) {
    try {
      const string& input_expr = read(in);
      if (in.eof()) {
        break;
      }
      result = interpret(input_expr);
    } 
    catch (const runtime_error& e) {
      cout << "\nERROR: " << e.what() << "\n";
      return; 
    }
  }
  display(result);
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
    cout << "Usage: ./interpreter [file-to-run]" << endl;
  }
}
