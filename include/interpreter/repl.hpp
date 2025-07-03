#include <string_view>
#include <vector>
#include <optional>

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
