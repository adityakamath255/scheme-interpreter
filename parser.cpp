#include "types.hpp"
#include "parser.hpp"
#include <cmath>
#include <string_view>

namespace Scheme {

static Obj*
make_num_obj(const std::string_view str) {
  return new Obj(stod(string(str)));
}

static Obj*
make_sym_obj(const std::string_view str) {
  return new Obj(Symbol(string(str)));
}

static Obj*
make_bool_obj(const std::string_view str) {
  if (str[1] == 't')
    return new Obj(true);
  else if (str[1] == 'f') 
    return new Obj(false);
  else 
    return make_sym_obj(str);
}

static Obj*
make_str_obj(const std::string_view str) {
  Obj *ret = new Obj(string(str.substr(1, str.size() - 2)));
  return ret;
}

static Obj*
from_str(const std::string_view str) {
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

Parser::Parser(const vector<std::string_view>& tokens):
  tokens {tokens},
  index {0}
{}

Obj*
Parser::parse_impl(bool recursive) {
  const auto token = tokens[index];
  Obj *head;

  index++;

  if (token == ")") {
    return new Obj(nullptr);
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
    head = new Obj(new Cons(
      *make_sym_obj("quote"), new Cons(
      *quoted, 
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
    return new Obj(new Cons(*head, *tail));
  }
}

Obj
Parser::parse() {
  return *parse_impl(true);
}

}
