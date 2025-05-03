#pragma once
#include "types.hpp"
#include <string>
#include <string_view>

namespace Scheme {

struct Token {
  enum Type {
    LPAREN,
    RPAREN,
    NUMBER,
    SYMBOL,
    STRING,
    VEC_BEGIN,
    TRUE,
    FALSE,
    PLUS_INF,
    PLUS_NAN,
    MINUS_INF,
    MINUS_NAN,
    DOT, 
    QUOTE,
    BACKTICK,
    COMMA,
    SPLICE_COMMA,
    END,
    ERROR
  } type;

  std::string_view lexeme;
};

class Lexer {
private:
  const std::string_view input;
  size_t start;
  size_t curr;

  size_t get_pos() const;
  bool at_end() const;
  bool at_boundary() const;
  char peek() const;
  char peek_next() const;
  char peek_prev() const;
  char advance();
  bool match(const char);
  bool match_exact_word(const std::string&);
  bool match_word(const std::string&);
  std::runtime_error error(const std::string&);
  void skip_semicolon_comment();
  void skip_hash_comment();
  bool skip_comment();
  bool skip_whitespace();
  void skip_whitespace_and_comments();
  Token make_token(Token::Type);
  Token hash_token();
  Token string_token();
  Token symbol_token();
  Token number_token();

public:
  Lexer(const std::string_view input);
  Token next_token();
  std::vector<Token> all_tokens();
};

}
