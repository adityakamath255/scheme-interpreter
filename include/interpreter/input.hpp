#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <interpreter.hpp>
#include <replxx.hxx>

namespace Scheme {

class BracketChecker {
private:
  enum class State {
    Initial,
    Term,
    Expression,
    String,
    Escaped,
    SeenHash,
    SeenPipe,
    BlockComment,
    Finished,
  };
  
  std::string_view line;
  std::vector<State> state_stk;
  std::vector<char> bracket_stk;
  size_t block_comment_depth;
  size_t pos;
  bool concluded;

  bool at_end() const;
  char advance();
  void retract();
  void skip_line();
  State curr_state();
  void push_state(const State);
  void pop_state();
  void set_state(const State);
  void push_bracket(const char);
  void pop_bracket(const char);
  void process();
  void process_initial();
  void process_term();
  void process_expression();
  void process_string();
  void process_escaped();
  void process_seen_hash();
  void process_seen_pipe();
  void process_block_comment();
  void process_finished();

public:
  BracketChecker();
  std::optional<size_t> read(const std::string_view);
};

class InputReader {
protected:
  virtual std::string& get_buffer_ref() = 0; // avoiding having a buffer field to make this class an abstract interface
  virtual std::optional<std::string> get_line() = 0;

public:
  virtual ~InputReader() = default;
  std::optional<std::string> get_expr();
  virtual void print_result(const Obj) = 0;
};

class FileReader : public InputReader {
private:
  std::string buffer;
  std::ifstream stream;
  bool enter_repl;

  std::string& get_buffer_ref() override { return buffer; }

public:
  FileReader(const char *, const bool);
  std::optional<std::string> get_line() override;
  void print_result(const Obj) override;
};


class Repl : public InputReader {
private:  
  std::string buffer;
  replxx::Replxx rx;

  std::string& get_buffer_ref() override { return buffer; }

public:
  Repl();
  std::optional<std::string> get_line() override;
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
