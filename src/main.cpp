#include <interpreter/types.hpp>
#include <interpreter/interpreter.hpp>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace Scheme;

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

    result << line << std::endl;

    if (open_parens == 0) {
      break;
    }
  }

  if (open_parens != 0) {
    throw std::runtime_error("unmatched parentheses at end of input");
  }

  return result.str();
}

void
driver_loop(Interpreter& interp) {
  while (true) {
    try {
      std::cout << ">>> ";
      std::string input_expr = read(std::cin);
      std::string trimmed;
      std::istringstream s(input_expr);
      s >> trimmed;
      if (trimmed == "#q" || trimmed == "(exit)") {
        return;
      }
      auto result = interp.interpret(input_expr);
      if (!is_void(result)) {
        std::cout << stringify(result) << std::endl;
      }
    } 
    catch (const std::runtime_error& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
    }
    catch (const std::bad_variant_access& e) {
      std::cerr << "ERROR: incorrect type" << std::endl;
    }
  }
}

void
run_file(Interpreter& interp, const char *filename, bool enter_driver_loop) {
  std::ifstream in(filename);
  if (!in.is_open()) {
      std::cerr << "ERROR: Could not open file '" << filename << "'" << std::endl;
      return;
  }
  while (true) {
    try {
      const std::string& input_expr = read(in);
      if (in.eof()) {
        break;
      }
      interp.interpret(input_expr);
    } 
    catch (std::runtime_error& e) {
      std::cerr << std::endl << "ERROR: " << e.what() << std::endl;
      return; 
    }
  }
  std::cout << std::endl;
  if (interp.is_profiled()) {
    std::cout << std::endl;
    interp.print_timings();
    std::cout << std::endl;
  }
  if (enter_driver_loop) {
    driver_loop(interp);
  }
}

int 
main(const int argc, const char **argv) {
  bool profiling = false;
  bool enter_repl = true;
  const char *filename = nullptr;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--profile") {
      profiling = true;
    }
    else if (arg == "--no-repl") {
      enter_repl = false;
    }
    else if (!filename) {
      filename = argv[i];
    }
    else {
      std::cerr << "Usage: ./scheme [--profile] [--no-repl] [filename]" << std::endl;
      return 1;
    }
  }
  
  Interpreter interp(profiling);

  if (!filename) {
    driver_loop(interp);
  }
  else {
    run_file(interp, filename, enter_repl);
  }
}
