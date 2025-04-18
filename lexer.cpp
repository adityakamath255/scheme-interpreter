#include "types.hpp"
#include "lexer.hpp"
#include <string_view>
#include <algorithm>

namespace Scheme {

static const char SPECIAL_CHARS[] = {'(', ')', '\'', '`', ',', '\"', ';'};

static bool
is_special(const char c) {
  return std::find(std::begin(SPECIAL_CHARS), std::end(SPECIAL_CHARS), c) != std::end(SPECIAL_CHARS);
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

vector<std::string_view>
Lexer::tokenize() {
  tokens.push_back("(");
  tokens.push_back("begin");

  bool is_string = false;

  while (curr < input.size()) {
    const char c = input[curr];
    if (c == '"') {
      if (is_string) {
        insert(true);
      }
      else {
        insert(false);
      }
      is_string = !is_string;
    }
    else if (!is_string) {
      if (isspace(c)) {
        insert(false);
        start++;
      }

      else if (is_special(c)) {
        insert(false);
        insert(true);
      }
    } 
    curr++;
  }

  insert(false);
  tokens.push_back(")");
  return tokens;
}

}
