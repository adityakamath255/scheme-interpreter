#include "types.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <string_view>

namespace Scheme {

Obj
Parser::make_num_obj(const std::string_view str) {
  double val;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc()) {
    throw std::runtime_error("failed to parse number");
  }
  return Obj(val);
}

Obj
Parser::make_sym_obj(const std::string_view str) {
  return Obj(interp.intern_symbol(str));
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
  return Obj(std::string(str.data() + 1, str.size() - 2));
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

Obj
Parser::parse_vec() {
  std::vector<Obj> ret {};
  while (index < tokens.size() && tokens[index] != ")") {
    ret.push_back(parse_impl(false));
  }
  if (index == tokens.size()) {
    throw std::runtime_error("unterminated vector");
  }
  else {
    index++;
    return interp.alloc.make<Vector>(std::move(ret));
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
    return nullptr;
  }
  else if (token == "(") {
    head = parse_impl(true);
  }
  else if (token == "#(") {
    head = parse_vec();
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
    head = Obj(interp.alloc.make<Cons>(
      make_sym_obj("quote"), interp.alloc.make<Cons>(
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
    return Obj(interp.alloc.make<Cons>(head, tail));
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
