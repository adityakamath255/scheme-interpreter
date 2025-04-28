#include "types.hpp"
#include "lexer.hpp"
#include <string_view>

namespace Scheme {

static bool
is_space(const char c) {
  switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      return true;
    default:
      return false;
  }
}

static bool
is_special(const char c) {
  switch (c) {
    case '(':
    case ')':
    case '\'':
    case '`':
    case ',':
    case ';':
      return true;
    default:
      return false;
  }
}

Lexer::Lexer(const std::string_view input): 
  input {input},
  start {0},
  curr {0},
  tokens {}
{}

void
Lexer::insert(const bool inclusive) {
  const size_t len = curr - start + (inclusive ? 1 : 0);
  if (len > 0) {
    auto tok = input.substr(start, len);
    tokens.push_back(tok);
    start += len;
  }
}

void 
Lexer::skip_whitespace() {
  do {
    curr++;
  } while (curr < input.size() && is_space(input[curr]));
  start = curr;
}

std::vector<std::string_view>
Lexer::tokenize() {
  bool is_string = false;

  while (curr < input.size()) {
    if (input[curr] == '"') {
      if (is_string) {
        insert(true);
      }
      else {
        insert(false);
      }
      is_string = !is_string;
    }
    else if (!is_string) {
      if (is_space(input[curr])) {
        insert(false);
        skip_whitespace();
        continue;
      }

      else if (is_special(input[curr])) {
        if (start == curr - 1 && input[start] == '#' && input[curr] == '(') {
          insert(true);
        }
        else {
          insert(false);
          insert(true);
        }
      }
    } 
    curr++;
  }

  insert(false);
  return tokens;
}

}
