#include <string_view>
#include <vector>
#include <optional>

class BracketChecker {
private:
  enum class State {
    Normal,
    String,
    Escaped,
    LineComment,
    SeenHashInNormal,
    SeenHashInBlockComment,
    SeenPipe,
    BlockComment,
    Finished,
  };
  
  std::string_view line;
  State state = State::Normal;
  std::vector<char> brackets;
  size_t block_comment_depth = 0;
  size_t pos = 0;

  std::optional<size_t> read();
  void reset();
  void set_state(const State);
  bool at_end() const;
  char advance();
  void retract();
  void skip_line();
  char peek() const;
  void push_bracket();
  void pop_bracket();
  void process();
  void process_normal();
  void process_string();
  void process_escaped();
  void process_line_comment();
  void process_seen_hash_from_normal();
  void process_seen_hash_from_block_comment();
  void process_seen_pipe();
  void process_block_comment();
  void process_finished();
};
