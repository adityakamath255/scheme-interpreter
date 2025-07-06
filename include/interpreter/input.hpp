#include <optional>
#include <string>

#include <interpreter.hpp>
#include <replxx.hxx>

namespace Scheme {

struct InputReader {
  virtual ~InputReader() = default;
  virtual std::optional<std::string> get_expr() = 0;
  virtual void print_result(const Obj) = 0;
};

class FileReader : public InputReader {
private:
  std::string file_data;
  size_t curr_index;

public:
  FileReader(const std::string&);
  std::optional<std::string> get_expr() override;
  void print_result(const Obj) override;
};

class Repl : public InputReader {
private:
  replxx::Replxx rx;
  std::optional<std::string> pending_input;

public:
  Repl();
  std::optional<std::string> get_expr() override;
  void print_result(const Obj) override;

};

class CompositeReader : public InputReader {
private:
  FileReader file_reader;
  Repl repl;
  bool file_done;

public:
  CompositeReader(const std::string&);
  std::optional<std::string> get_expr() override;
  void print_result(const Obj) override;
};

class Session {
private:
  std::unique_ptr<InputReader> input;
  std::unique_ptr<Interpreter> interp;

public:
  Session(std::unique_ptr<InputReader> input, std::unique_ptr<Interpreter> interp);
  void run();
};

}
