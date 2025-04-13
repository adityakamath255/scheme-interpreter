#include "parsing.hpp"
#include <cmath>

namespace Scheme {

static const vector<char> SPECIAL_CHARS {'(', ')', '\'', '`', ',', '\"', ';'};

vector<string>
tokenize(const string& input) {
  vector<string> tokens {"(", "begin"};

  const auto insert_and_clear = [&](string *token) -> void {
    if (!token->empty()) {
      tokens.push_back(*token);
      token->clear();
    }
  };

  string token {};
  bool is_string = 0;

  for (const char c : input) {
    if (c == '"') {
      if (is_string) {
        token += "\"";
        insert_and_clear(&token);
        is_string = false;
      }
      else {
        insert_and_clear(&token);
        token += "\"";
        is_string = true;
      }
    }
    else if (!is_string) {
      if (isspace(c)) {
        insert_and_clear(&token);
      }

      else if (std::find(SPECIAL_CHARS.begin(), SPECIAL_CHARS.end(), c) != SPECIAL_CHARS.end()) {
        insert_and_clear(&token);
        tokens.push_back(string(1, c));
      }

      else {
        token += c;
      } 

    } 
    else {
      token += c;
    }
  }

  if (!token.empty())
    insert_and_clear(&token);

  tokens.push_back(")");
  return tokens;
};

static Obj *
make_num_obj(const string& str) {
  return new Obj(stod(str));
}

static Obj *
make_sym_obj(const string& str) {
  return new Obj(Symbol(str));
}

static Obj *
make_bool_obj(const string& str) {
  if (str[1] == 't')
    return new Obj(true);
  else if (str[1] == 'f') 
    return new Obj(false);
  else 
    return make_sym_obj(str);
}

static Obj *
make_str_obj(const string& str) {
  Obj *ret = new Obj(str.substr(1, str.size() - 2));
  return ret;
}

static Obj *
from_str(const string& str) {
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

static std::pair<Obj*, int> 
parse_impl(const vector<string>& tokens, const int curr_index, bool recursive) {
  const string& token = tokens[curr_index];
  Obj *head;
  int next_index;

  if (token[0] == ')')  {
    return {new Obj(nullptr), curr_index + 1};
  }
  
  else if (token[0] == '(') {
    tie(head, next_index) = parse_impl(tokens, curr_index + 1, true);
  }

  else if (token[0] == '.') {
    tie(head, next_index) = parse_impl(tokens, curr_index + 1, false);
    if (next_index >= tokens.size() || tokens[next_index] != ")") {
      throw runtime_error("bad positioning of '.'");
    }
    next_index++;
    recursive = 0;
  }

  else if (token[0] == '\'') {
    Obj *q;
    tie(q, next_index) = parse_impl(tokens, curr_index + 1, false);
    head = new Obj(
      new Cons(
        *make_sym_obj("quote"), 
        new Cons(
          *q, 
          nullptr)));
  }

  else {
    tie(head, next_index) = std::make_pair(from_str(token), curr_index + 1);
  }

  if (next_index == tokens.size() || !recursive) {
    return {head, next_index};
  }

  else {
    if (recursive) {
      const auto [tail, k] = parse_impl(tokens, next_index, 1);
      Cons *c = new Cons(*head, *tail);
      return {new Obj(c), k};
    }
    else {
      throw runtime_error("ill-formed syntax");
      return {nullptr, 0};
    }
  }
}

Obj
parse(const vector<string>& tokens) {
  auto ret = parse_impl(tokens, 0, 1).first;
  return *ret;
}

}
