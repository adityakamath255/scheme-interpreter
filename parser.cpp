#include "types.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <cmath>
#include <string_view>

namespace Scheme {

Obj
Parser::make_num_obj(const std::string_view str) {
  return Obj(stod(std::string(str)));
}

Obj
Parser::make_sym_obj(const std::string_view str) {
  return Obj(interp.intern_symbol(std::string(str)));
}

Obj
Parser::make_bool_obj(const std::string_view str) {
  if (str[1] == 't')
    return Obj(true);
  else if (str[1] == 'f') 
    return Obj(false);
  else 
    return make_sym_obj(str);
}

Obj
Parser::make_str_obj(const std::string_view str) {
  return Obj(std::string(str.substr(1, str.size() - 2)));
}

Obj
Parser::from_str(const std::string_view str) {
  if (str[0] == '#') {
    return make_bool_obj(str);
  }
  else if (str[0] == '"') {
    return make_str_obj(str);
  }
  else if (str[0] == '-') {
    if (str.size() != 1 && isdigit(str[1]))
      return make_num_obj(str);
    else
      return make_sym_obj(str);
  }
  else if (isdigit(str[0])) {
      return make_num_obj(str);
  } 
  else {
    return make_sym_obj(str);
  }
} 

Parser::Parser(const std::vector<std::string_view>& tokens, Interpreter& interp):
  tokens {tokens},
  index {0},
  interp {interp}
{}

Obj
Parser::parse_impl(bool recursive) {
  const auto token = tokens[index];
  Obj head;

  index++;

  if (token == ")") {
    return Obj(nullptr);
  }
  else if (token == "(") {
    head = parse_impl(true);
  }
  else if (token == ".") {
    head = parse_impl(false);
    if (index >= tokens.size() || tokens[index] != ")") {
      throw std::runtime_error("bad positioning of '.'");
    }
    index++;
    recursive = false;
  }
  else if (token == "'") {
    auto quoted = parse_impl(false);
    head = Obj(interp.alloc.make_cons(
      make_sym_obj("quote"), interp.alloc.make_cons(
      quoted, 
      nullptr)));
  }
  else {
    head = from_str(token);
  }

  if (index == tokens.size() || !recursive) {
    return head;
  }
  else {
    auto tail = parse_impl(true);
    return Obj(interp.alloc.make_cons(head, tail));
  }
}

Obj
Parser::parse() {
  if (tokens.empty()) {
    return Void {};
  }
  return parse_impl(false);
}

}
