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
  curr_tok {},
  tokens {}
{}

void
Lexer::insert_and_clear() {
  if (!curr_tok.empty()) {
    tokens.push_back(curr_tok);
    curr_tok.clear();
  }
}

vector<string>
Lexer::tokenize() {
  tokens.push_back("(");
  tokens.push_back("begin");

  bool is_string = 0;

  for (const char c : input) {
    if (c == '"') {
      if (is_string) {
        curr_tok += c;
        insert_and_clear();
      }
      else {
        insert_and_clear();
        curr_tok += c;
      }
      is_string = !is_string;
    }
    else if (!is_string) {
      if (isspace(c)) {
        insert_and_clear();
      }

      else if (is_special(c)) {
        insert_and_clear();
        tokens.push_back(string(1, c));
      }

      else {
        curr_tok += c;
      } 

    } 
    else {
      curr_tok += c;
    }
  }

  if (!curr_tok.empty())
    insert_and_clear();

  tokens.push_back(")");
  return tokens;
}

}
