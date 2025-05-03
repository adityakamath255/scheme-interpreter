#include "types.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <string_view>

namespace Scheme {

Parser::Parser(const std::vector<std::string_view>& tokens, Interpreter& interp):
  tokens {tokens},
  index {0},
  interp {interp}
{}

std::string_view 
Parser::next_token() {
  if (index >= tokens.size()) {
    return "\0"; // replace with Token::EOF
  }
  else {
    index++;
    return tokens[index - 1];
  }
}

bool
Parser::match(const std::string_view str) {
  if (index >= tokens.size()) {
    return false;
  }
  else if (tokens[index] == str) {
    index++;
    return true;
  }
  else {
    return false;
  }
}

Obj
Parser::make_sym_obj(const std::string_view str) {
  return interp.intern_symbol(str);
}

Obj
Parser::make_bool_obj(const std::string_view str) {
  if (str.size() != 2) {
    throw std::runtime_error("identifiers cannot start with '#'");
  }
  switch (str[1]) {
    case 't': 
    case 'T':
      return true;

    case 'f':
    case 'F':
      return false;

    default:
      throw std::runtime_error("identifiers cannot start with '#'");
  }
}

Obj
Parser::make_num_obj(const std::string_view str) {
  try {
    size_t chars_processed;
    const double val = std::stod(std::string(str), &chars_processed);

    if (chars_processed == str.size()) {
      return val;
    }
    else {
      return make_sym_obj(str);
    }
  }
  catch (const std::exception&) {
    return make_sym_obj(str);
  }
}

Obj
Parser::make_str_obj(const std::string_view str) {
  return std::string(str.data() + 1, str.size() - 2);
}

Obj
Parser::from_str(const std::string_view str) {
  switch (str[0]) {
    case '#':
      return make_bool_obj(str);
    case '"':
      return make_str_obj(str);
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9':
    case '+': case '-':
      return make_num_obj(str);
    default:
      return make_sym_obj(str);
  }
} 

Obj 
Parser::parse_atom() {
  const auto token = next_token();

  if (token == ")") {
    return nullptr;
  }
  else if (token == "(") {
    return parse_list();
  }
  else if (token == "#(") {
    return parse_vec();
  }
  else if (token == ".") {
    return parse_dotted_tail();
  }
  else if (token == "'") {
    return parse_quoted("quote");
  }
  else if (token == "`") {
    return parse_quoted("quasiquote");
  }
  else if (token == ",") {
    return parse_quoted("unquote");
  }
  else if (token == ",@") {
    return parse_quoted("unquote-splicing");
  }
  else {
    return from_str(token);
  }
}

Obj
Parser::parse_list() {
  if (match(")")) {
    return nullptr;
  }
  if (match(".")) {
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
  while (!match(")")) {
    ret.push_back(parse_atom());
  }
  return interp.spawn<Vector>(std::move(ret));
}

Obj 
Parser::parse_dotted_tail() {
  if (match(")")) {
    throw std::runtime_error("bad positioning of '.'");
  }
  const auto ret = parse_atom();
  if (!match(")")) {
    throw std::runtime_error("bad positioning of '.'");
  }
  return ret;
}

Obj
Parser::parse_quoted(const std::string_view quote_type) {
  const auto quoted = parse_atom();
  return Obj(interp.spawn<Cons>(
    make_sym_obj(quote_type), interp.spawn<Cons>(
    quoted, 
    nullptr)));
}

Obj
Parser::parse() {
  if (tokens.empty()) {
    return Void {};
  }
  return parse_atom();
}

}
