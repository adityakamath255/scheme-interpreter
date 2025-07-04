#include <interpreter/input.hpp>
#include <interpreter/interpreter.hpp>
#include <interpreter/types.hpp>
#include <iostream>

using namespace Scheme;

std::unique_ptr<InputReader>
make_reader(const char *filename) {
  if (filename) {
    return std::make_unique<FileReader>(filename, false);
  }
  else {
    return std::make_unique<Repl>();
  }
}

Session
make_session(const bool profiling, const bool enter_repl, const char *filename) {
  return Session(
    make_reader(filename), 
    std::make_unique<Interpreter>(profiling)
  );
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
  
  auto session = make_session(profiling, enter_repl, filename);
  session.run();
}
