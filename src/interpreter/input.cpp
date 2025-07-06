#include <iostream>
#include <fstream>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string_view>
#include <input.hpp>
#include <interpreter.hpp>
#include <types.hpp>

namespace Scheme {

constexpr int REPL_MAX_HISTORY_SIZE = 1000;

char 
expected_closing(const char opening) {
  return opening == '(' ? ')' : ']';
}

class BracketChecker {
private:
  std::string_view input;

  size_t 
  skip_whitespace(size_t pos) const {
    while (pos < input.size()) {
      const char c = input[pos];

      switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
          pos += 1;
          break;

        case ';':
          pos = skip_line_comment(pos);
          break;

        default:
          return pos;
      }
    }

    return pos;
  }

  size_t 
  parse_term(size_t pos) const {
    while (pos < input.size()) {
      switch (input[pos]) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case ';':
        case '"':
        case '(':
        case '[':
        case ')':
        case ']':
          return pos;

        default:
          pos += 1;

      }
    }

    return pos;
  }

  std::optional<size_t> 
  parse_expr(size_t pos) const {
    const char opening = input[pos];
    const char closing = expected_closing(opening);
    std::stack<char> bracket_stk;
    bracket_stk.push(closing);

    pos += 1;

    while (pos < input.size() && !bracket_stk.empty()) {
      pos = skip_whitespace(pos);
      if (pos >= input.size()) {
        return std::nullopt;
      }

      const char c = input[pos];

      switch (c) {
        case ')':
        case ']':
          if (bracket_stk.empty() || c != bracket_stk.top()) {
            throw std::runtime_error("mismatched brackets");
          }

          bracket_stk.pop();

          pos += 1;

          if (bracket_stk.empty()) {
            return pos;
          }

          break;

        case ';':
          pos = skip_line_comment(pos);
          break;

        case '"':
          if (const auto string_end = parse_str(pos)) {
            pos = *string_end;
          }
          else {
            return std::nullopt;
          }
          break;

        case '#':
          if (pos + 1 < input.size() && input[pos + 1] == '|') {
            if (const auto comment_end = parse_block_comment(pos)) {
              pos = *comment_end;
            }
            else {
              return std::nullopt;
            }
          }
          break;

        case '(':
        case '[':
          bracket_stk.push(expected_closing(c));
          pos += 1;
          break;

        default:
          pos = parse_term(pos);
      }
    }

    return 
      bracket_stk.empty() ?
      std::optional<size_t>(pos) :
      std::nullopt;
  }

  std::optional<size_t> 
  parse_str(size_t pos) const {
    pos += 1;

    while (pos < input.size()) {
      const char c = input[pos];

      if (c == '"') {
        return pos + 1;
      }
      else if (c == '\\' && pos + 1 < input.size()) {
        pos += 2;
      }
      else {
        pos += 1;
      }
    }

    return std::nullopt;
  }

  std::optional<size_t> 
  parse_block_comment(size_t pos) const {
    pos += 2;
    int depth = 1;

    while (pos < input.size() && depth > 0) {
      if (pos + 1 < input.size()) {
        if (input[pos] == '#' && input[pos + 1] == '|') {
          depth += 1;
          pos += 2;
        }
        else if (input[pos] == '|' && input[pos + 1] == '#') {
          depth -= 1;
          pos += 2;
        }
        else {
          pos += 1;
        }
      }
      else {
        pos += 1;
      }
    }

    return 
      depth == 0 ? 
      std::optional<size_t>(pos) : 
      std::nullopt;
  }

  size_t 
  skip_line_comment(size_t pos) const {
    while (pos < input.size() && input[pos] != '\n') {
      pos += 1;
    }
    return pos;
  }


public:
  BracketChecker(const std::string_view input): input {input} {}

  std::optional<size_t> 
  check() const {
    size_t pos = 0;

    pos = skip_whitespace(pos);

    if (pos >= input.size()) {
      return std::nullopt;
    }
    else {
      const char c = input[pos];

      if (c == '(' || c == '[') {
        return parse_expr(pos);
      }
      else if (c == '#' && pos + 1 < input.size() && input[pos + 1] == '|') {
        return parse_block_comment(pos);
      }
      else if (c == '"') {
        return parse_str(pos);
      }
      else {
        return parse_term(pos);
      }
    }
  }
};

FileReader::FileReader(const std::string& file_name, const bool enter_repl): 
  curr_index {0},
  enter_repl {enter_repl}
{
  std::ifstream is(file_name);
  if (!is.is_open()) {
    throw std::runtime_error("error opening file: " + std::string(file_name));
  }
  std::ostringstream os;
  os << is.rdbuf();
  file_data = os.str();
}

std::optional<std::string>
FileReader::get_expr() {
  const std::string_view view = file_data.substr(curr_index);
  if (const auto next_index = BracketChecker(view).check()) {
    const std::string ret = file_data.substr(curr_index, *next_index);
    curr_index = *next_index;
    return ret;
  }
  else {
    return std::nullopt;
  }
}

void
FileReader::print_result(const Obj) {}

Repl::Repl():
  rx(),
  pending_input {std::nullopt}
{
  rx.set_max_history_size(REPL_MAX_HISTORY_SIZE);
  rx.set_word_break_characters(" \t\r()[]\"';");
}

std::optional<std::string>
Repl::get_expr() {
  std::string buffer;
  std::string prompt = ">> ";

  while (true) {
    if (pending_input) {
      rx.set_preload_buffer(*pending_input);
      pending_input = std::nullopt;
    }

    if (char const *line_cstr = rx.input(prompt)) {
      std::string line(line_cstr);
      rx.history_add(line);

      if (!buffer.empty()) {
        buffer += "\n";
      }

      const size_t start = buffer.find_first_not_of(" \t\r\n");
      if (start == std::string::npos) {
        buffer.clear();
      }

      else {
        std::string_view trimmed = buffer.substr(start);
        BracketChecker checker(trimmed);

        if (const auto result = checker.check()) {
          const size_t expr_end = start + result.value();
          const std::string complete_expr = buffer.substr(start, result.value());
          const size_t next_start = buffer.find_first_not_of(" \t\r\n", expr_end);

          if (next_start != std::string::npos) {
            pending_input = buffer.substr(next_start);
          }

          return complete_expr;
        }

        else {
          buffer += line;
        }

        prompt = "..";
      }
    }
    else {
      if (!buffer.empty()) {
        throw std::runtime_error("unexpected EOF; incomplete expression");
      }
      return std::nullopt;
    }
  }
}

void
Repl::print_result(const Obj result) {
  if (!is_void(result)) {
    std::cout << stringify(result) << std::endl;
  }
}

Session::Session(std::unique_ptr<InputReader> input, std::unique_ptr<Interpreter> interp):
  input {std::move(input)},
  interp {std::move(interp)}
{}

void
Session::run() {
  std::cout << "Scheme Interpreter - Press Ctrl+D to exit, Ctrl+C to clear line" << std::endl;

  while (const auto expr = input->get_expr()) {
    try {
      const Obj result = interp->interpret(*expr);
      input->print_result(result);
    }
    catch (const std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
    }
  }

  std::cout << "\nExiting..." << std::endl;
}

}
