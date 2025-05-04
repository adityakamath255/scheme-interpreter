#include "types.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <cmath>
#include <string_view>

namespace Scheme {

Parser::Parser(const std::vector<Token>& tokens, Interpreter& interp):
  tokens {tokens},
  index {0},
  interp {interp}
{}

bool
Parser::at_end() {
  return index >= tokens.size();
}

const Token&
Parser::advance() {
  index++;
  return tokens[index - 1];
}

const Token&
Parser::curr_token() {
  return tokens[index - 1];
}

const Token&
Parser::next_token() {
  return tokens[index];
}

bool
Parser::match(const Token::Type type) {
  if (at_end()) {
    return false;
  }
  else if (next_token().type == type) {
    advance();
    return true;
  }
  else {
    return false;
  }
}

Obj
Parser::symbol() {
  return interp.intern_symbol(curr_token().lexeme);
}

Obj
Parser::number() {
  try {
    size_t chars_processed;
    const double val = std::stod(std::string(curr_token().lexeme), &chars_processed);

    if (chars_processed == curr_token().lexeme.size()) {
      return val;
    }
    else {
      return symbol();
    }
  }
  catch (const std::exception& e) {
    return symbol();
  }
}

Obj
Parser::string() {
  return interp.spawn<String>(std::string(curr_token().lexeme));
}

Obj 
Parser::parse_atom() {
  advance();
  switch (curr_token().type) {
    case Token::LPAREN:
      return parse_list(); 
      
    case Token::RPAREN:
      throw std::runtime_error("unexpected ')'");

    case Token::VEC_BEGIN:
      return parse_vec();
      
    case Token::DOT:
      return parse_dotted_tail();
      
    case Token::QUOTE:
      return parse_quoted("quote");
      
    case Token::BACKTICK:
      return parse_quoted("quasiquote");
      
    case Token::COMMA:
      return parse_quoted("unquote");
      
    case Token::SPLICE_COMMA:
      return parse_quoted("unquote-splicing");
      
    case Token::TRUE:
      return true;
      
    case Token::FALSE:
      return false;
      
    case Token::PLUS_INF:
      return std::numeric_limits<double>::infinity();
      
    case Token::MINUS_INF:
      return -std::numeric_limits<double>::infinity();
      
    case Token::PLUS_NAN:
    case Token::MINUS_NAN:
      return std::numeric_limits<double>::quiet_NaN();
      
    case Token::NUMBER:
      return number();
      
    case Token::STRING:
      return string();
      
    case Token::SYMBOL:
      return symbol();

    case Token::END: 
    case Token::ERROR: 
    default:
      return Void {};
  }
}

Obj
Parser::parse_list() {
  if (match(Token::RPAREN)) {
    return Null {};
  }
  if (match(Token::DOT)) {
    return parse_dotted_tail();
  }
  else {
    const auto head = parse_atom();
    const auto tail = parse_list();
    return Obj(interp.spawn<Cons>(head, tail));
  }
}

Obj
Parser::parse_vec() {
  std::vector<Obj> ret {};
  while (!match(Token::RPAREN)) {
    ret.push_back(parse_atom());
  }
  return interp.spawn<Vector>(std::move(ret));
}

Obj 
Parser::parse_dotted_tail() {
  if (match(Token::RPAREN)) {
    throw std::runtime_error("bad positioning of '.'");
  }
  const auto ret = parse_atom();
  if (!match(Token::RPAREN)) {
    throw std::runtime_error("bad positioning of '.'");
  }
  return ret;
}

Obj
Parser::parse_quoted(const std::string& quote_type) {
  const auto sym = interp.intern_symbol(quote_type);
  const auto quoted = parse_atom();
  return Obj(interp.spawn<Cons>(
    sym, interp.spawn<Cons>(
    quoted, 
    Null {})));
}

Obj
Parser::parse() {
  if (tokens.empty()) {
    return Void {};
  }
  return parse_atom();
}

}
