#include <interpreter/input.hpp>
#include <interpreter/interpreter.hpp>
#include <interpreter/types.hpp>
#include <iostream>

using namespace Scheme;

std::unique_ptr<InputReader>
make_reader(const std::optional<std::string>& filename, const bool enter_repl) {
  if (filename) {
    if (enter_repl) {
      return std::make_unique<CompositeReader>(*filename);
    }
    else {
      return std::make_unique<FileReader>(*filename);
    }
  }
  else {
    return std::make_unique<Repl>();
  }
}

Session
make_session(const bool profiling, const bool enter_repl, const std::optional<std::string>& filename) {
  return Session(
    make_reader(filename, enter_repl), 
    std::make_unique<Interpreter>(profiling)
  );
}

int 
main(const int argc, const char **argv) {
  bool profiling = false;
  bool enter_repl = true;
  std::optional<std::string> filename = std::nullopt;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--profile") {
      profiling = true;
    }
    else if (arg == "--no-repl") {
      enter_repl = false;
    }
    else if (!filename) {
      filename = std::string(argv[i]);
    }
    else {
      std::cerr << "Usage: ./scheme [--profile] [--no-repl] [filename]" << std::endl;
      return 1;
    }
  }
  
  auto session = make_session(profiling, enter_repl, filename);
  session.run();
}
