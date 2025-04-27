#include "types.hpp"
#include "interpreter.hpp"
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
driver_loop(Interpreter& interp) {
  while (true) {
    try {
      std::cout << ">>> ";
      std::string input_expr = read(std::cin);
      if (input_expr == "exit\n") 
        return;
      auto result = interp.interpret(input_expr);
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
run_file(Interpreter& interp, const char *filename, bool enter_driver_loop) {
  std::ifstream in;
  in.open(filename);
  while (true) {
    try {
      const std::string& input_expr = read(in);
      if (in.eof()) {
        break;
      }
      interp.interpret(input_expr);
    } 
    catch (std::runtime_error& e) {
      std::cerr << "\nERROR: " << e.what() << "\n";
      return; 
    }
  }
  std::cout << "\n";
  if (interp.is_profiled()) {
    std::cout << "\n";
    interp.print_timings();
    std::cout << "\n";
  }
  if (enter_driver_loop) {
    driver_loop(interp);
  }
}

int 
main(const int argc, const char **argv) {
  Interpreter interp(true);
  if (argc == 1) {
    driver_loop(interp);
  }
  else if (argc == 2) {
    run_file(interp, argv[1], true);
  }
  else if (argc == 3 && std::string(argv[1]) == "--no-repl") {
    run_file(interp, argv[2], false);
  }
  else {
    std::cout << "Usage: ./scheme [--no-repl] [file-to-run]" << std::endl;
  }
}
