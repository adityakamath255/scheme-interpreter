#include "types.hpp"
#include "lexer.hpp"
#include <algorithm>

namespace Scheme {

static const char SPECIAL_CHARS[] = {'(', ')', '\'', '`', ',', '\"', ';'};

static bool
is_special(const char c) {
  return std::find(std::begin(SPECIAL_CHARS), std::end(SPECIAL_CHARS), c) != std::end(SPECIAL_CHARS);
}

Lexer::Lexer(const string& input): 
  input {input},
  start {0},
  curr {0},
  tokens {}
{}

void
Lexer::insert(const bool inclusive) {
  const size_t substr_size = curr - start + inclusive;
  if (substr_size > 0) {
    const string curr_tok = input.substr(start, substr_size);
    if (!curr_tok.empty()) {
      tokens.push_back(curr_tok);
    }
  }
  start += substr_size;
}

vector<string>
Lexer::tokenize() {
  tokens.push_back("(");
  tokens.push_back("begin");

  bool is_string = 0;

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
