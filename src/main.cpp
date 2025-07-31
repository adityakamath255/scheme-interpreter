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

void 
print_help() {
  std::cout << "Scheme Interpreter\n\n";
  std::cout << "Usage: ./scheme [options] [filename]\n\n";
  std::cout << "Options:\n";
  std::cout << "  -h, --help     Show this help message\n";
  std::cout << "  -p, --profile  Enable profiling (show timing information)\n";
  std::cout << "  -b, --batch    Run in batch mode (no REPL after script)\n\n";
  std::cout << "Examples:\n";
  std::cout << "  ./scheme                    Start interactive REPL\n";
  std::cout << "  ./scheme script.scm         Run script then enter REPL\n";
  std::cout << "  ./scheme -b script.scm      Run script in batch mode\n";
  std::cout << "  ./scheme -p script.scm      Run script with profiling\n";
}

int 
main(const int argc, const char **argv) {
  bool profiling = false;
  bool enter_repl = true;
  std::optional<std::string> filename = std::nullopt;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--profile" || arg == "-p") {
      profiling = true;
    }
    else if (arg == "--batch" || arg == "-b") {
      enter_repl = false;
    }
    else if (arg == "--help" || arg == "-h") {
      print_help();
      return 0;
    }
    else if (!filename) {
      filename = std::string(argv[i]);
    }
    else {
      std::cerr << "Usage: ./scheme [options] [filename]" << std::endl;
      std::cerr << "Use --help for more information." << std::endl;
      return 1;
    }
  }
  
  auto session = make_session(profiling, enter_repl, filename);
  session.run();
}
