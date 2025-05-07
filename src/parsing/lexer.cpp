#include <parsing/lexer.hpp>
#include <stdexcept>
#include <vector>
#include <string>
#include <string_view>
#include <format>

namespace Scheme {

static bool
is_space(const char c) {
  switch (c) {
    case ' ': case '\t': case '\r': case '\n':
      return true;
    default:
      return false;
  }
}

static bool
is_digit(const char c) {
  switch (c) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return true;
    default:
      return false;
  }
}

static bool
is_special(const char c) {
  switch (c) {
    case '(': case ')': case '[': case ']':
    case '\'': case '"': case '`': case ',':
    case ';': case '#': case '\0': case EOF:
      return true;
    default:
      return false;
  }
}

static bool
is_boundary(const char c) {
  return is_space(c) || is_special(c);
}

Lexer::Lexer(const std::string_view input):
  input {input},
  start {0},
  curr {0}
{}

size_t 
Lexer::get_pos() const {
  return curr;
}

bool
Lexer::at_end() const  {
  return curr >= input.size();
}

bool
Lexer::at_boundary() const {
  return curr >= input.size() || is_boundary(input[curr]);
}

char
Lexer::peek() const {
  if (curr < input.size()) {
    return input[curr];
  }
  else {
    return '\0';
  }
}

char 
Lexer::peek_next() const {
  if (curr + 1 < input.size()) {
    return input[curr + 1];
  }
  else {
    return '\0';
  }
}

char 
Lexer::peek_prev() const {
  if (curr > 0) {
    return input[curr - 1];
  }
  else {
    return '\0';
  }
}

char
Lexer::advance() {
  curr++;
  return input[curr - 1];
}

bool
Lexer::match(const char expected) {
  if (at_end() || peek() != expected) {
    return false;
  }
  else {
    curr++;
    return true;
  }
}

// only match word if it ends at a boundary
bool
Lexer::match_exact_word(const std::string& expected) {
  size_t i;
  for (i = 0; curr + i < input.size() && i < expected.size(); i++) {
    if (input[curr + i] != expected[i]) {
      break; 
    }
  }
  if (i != expected.size() || !is_boundary(input[curr + i])) { 
    return false;
  }
  else {
    curr += expected.size();
    return true;
  }
}

// match word whether or not it ends at a boundary
bool
Lexer::match_word(const std::string& expected) {
  size_t i;
  for (i = 0; curr + i < input.size() && i < expected.size(); i++) {
    if (input[curr + i] != expected[i]) {
      break; 
    }
  }
  if (i != expected.size()) { 
    return false;
  }
  else {
    curr += expected.size();
    return true;
  }
}
std::runtime_error
Lexer::error(const std::string& err) {
  const auto tok = make_token(Token::ERROR);
  return std::runtime_error(std::format("{}: {}", err, tok.lexeme));
}

void 
Lexer::skip_semicolon_comment() {
  while (!at_end()) {
    if (match('\n')) {
      return;
    }
    else {
      advance();
    } 
  }
}

void 
Lexer::skip_hash_comment() {
  while (!at_end()) {
    if (match_word("|#")) {
      return;
    }
    else {
      advance();
    }
  }
  throw std::runtime_error("unterminated comment");
}

bool
Lexer::skip_comment() {
  if (match(';')) {
    skip_semicolon_comment();
    return true;
  }
  else if (match_word("#|")) {
    skip_hash_comment();
    return true;
  }   
  else {
    return false;
  }
}

bool
Lexer::skip_whitespace() {
  bool skipped = false;
  while (!at_end() && is_space(peek())) {
    skipped = true;
    advance();
  }
  return skipped;
}

void
Lexer::skip_whitespace_and_comments() {
  bool skipped_whitespace;
  bool skipped_comment;
  do {
    skipped_whitespace = skip_whitespace();
    skipped_comment = skip_comment();
  } while (skipped_whitespace || skipped_comment);
}

Token 
Lexer::make_token(const Token::Type type) {
  if (curr >= start) {
    const Token tok {type, input.substr(start, curr - start)};
    start = curr;
    return tok;
  }
  else {
    throw std::runtime_error(std::format("token of negative length at position {}", curr));
  }
}

Token
Lexer::hash_token() {
  if (match_exact_word("t") || match_exact_word("T")) {
    return make_token(Token::TRUE);
  }
  else if (match_exact_word("f") || match_exact_word("F")) {
    return make_token(Token::FALSE);
  }
  else if (match('(')) {
    return make_token(Token::VEC_BEGIN);
  }
  else if (match('\\')) {
    return char_token();
  }
  else {
    throw error("unidentified constant");
  }
} 

Token
Lexer::char_token() {
  start += 2;
  while (!at_boundary()) {
    advance();
  }
  return make_token(Token::CHAR);
}

Token
Lexer::string_token() {
  start++;
  while (!at_end()) {
    if (match('\\')) {
      if (!at_end()) {
        advance();
      }
    }
    else if (peek() == '"') {
      const auto tok = make_token(Token::STRING);
      curr++;
      return tok;
    }
    else {
      advance();
    }
  }
  throw error("unterminated string");
}

Token
Lexer::symbol_token() {
  while (!at_boundary()) {
    advance();
  }
  return make_token(Token::SYMBOL);
}

Token
Lexer::number_token() { 
  if (peek_prev() == '-' && match_exact_word("inf.0")) {
    return make_token(Token::MINUS_INF);
  }
  if (peek_prev() == '-' && match_exact_word("nan.0")) {
    return make_token(Token::MINUS_NAN);
  }
  if (peek_prev() == '+' && match_exact_word("inf.0")) {
    return make_token(Token::PLUS_INF);
  }
  if (peek_prev() == '+' && match_exact_word("nan.0")) {
    return make_token(Token::PLUS_NAN);
  }
  curr--;
  bool has_digits = false;
  while (!at_boundary()) {
    const char c = advance();
    if (is_digit(c)) {
      has_digits = true;
    }
    else {
      switch (c) {
        case '.':
        case '+': case '-':
        case 'e': case 'E':
          break;

        default:
          return symbol_token();
      }
    }
  }
  if (!has_digits) {
    return make_token(Token::SYMBOL);
  }
  else {
    return make_token(Token::NUMBER);
  }
}

Token
Lexer::next_token() {
  skip_whitespace_and_comments();
  start = curr;
  if (at_end()) {
    return make_token(Token::END);
  }
  switch (advance()) {
    case '(': case '[':
      return make_token(Token::LPAREN);
    case ')': case ']':
      return make_token(Token::RPAREN);
    case '\'':
      return make_token(Token::QUOTE);
    case '`':
      return make_token(Token::BACKTICK);
    case ',':
      if (match('@')) {
        return make_token(Token::SPLICE_COMMA);
      }
      else {
        return make_token(Token::COMMA);
      }
    case '.':
      if (at_boundary()) {
        return make_token(Token::DOT);
      }
      else {
        return number_token();
      }
    case '#':
      return hash_token();
    case '"':
      return string_token();
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case '+': case '-':
      return number_token();
    default:
      return symbol_token();
  }
}

std::vector<Token>
Lexer::all_tokens() {
  std::vector<Token> ret {};
  do {
    ret.push_back(next_token());
  } while (ret.back().type != Token::END);
  return ret;
}

}
