#include "interface.hpp"

namespace scheme {

const vector<char> SPECIAL_CHARS {'(', ')', '\'', '`', ',', '\"', ';'};

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
      insert_and_clear(&token);
      tokens.push_back("\"");
      is_string = !is_string;
    }
    else if (!is_string) {
      if (isspace(c)) {
        insert_and_clear(&token);
      }

      else if (find(SPECIAL_CHARS.begin(), SPECIAL_CHARS.end(), c) != SPECIAL_CHARS.end()) {
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

sc_obj *
make_bool_obj(const string& str) {
  if (str[1] == 't')
    return new sc_obj(true);
  else
    return new sc_obj(false);
}

sc_obj *
make_num_obj(const string& str) {
  return new sc_obj(stod(str));
}

sc_obj *
make_sym_obj(const string& str) {
  return new sc_obj(symbol(str));
}

sc_obj *
make_str_obj(const string& str) {
  return new sc_obj(str);
}

sc_obj *
sc_obj_from_str(const string& str) {
  if (str[0] == '#') {
    return make_bool_obj(str);
  }
  else if (str[0] == '"') {
    return make_str_obj(str);
  }
  else if (str[0] == '-') {
    if (str.size() != 0 && isdigit(str[1]))
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

pair<sc_obj*, int> 
parse_impl(const vector<string>& tokens, const int curr_index, bool recursive) {
  const string& token = tokens[curr_index];
  sc_obj *head;
  int next_index;

  if (token[0] == ')')  {
    return {new sc_obj(nullptr), curr_index + 1};
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
    sc_obj *q;
    tie(q, next_index) = parse_impl(tokens, curr_index + 1, false);
    head = new sc_obj(
      new cons(
        *make_sym_obj("quote"), 
        new cons(
          *q, 
          nullptr)));
  }

  else {
    tie(head, next_index) = make_pair(sc_obj_from_str(token), curr_index + 1);
  }

  if (next_index == tokens.size() || !recursive) {
    return {head, next_index};
  }

  else {
    if (recursive) {
      const auto [tail, k] = parse_impl(tokens, next_index, 1);
      cons *c = new cons(*head, *tail);
      return {new sc_obj(c), k};
    }
    else {
      throw runtime_error("ill-formed syntax");
      return {nullptr, 0};
    }
  }
}

sc_obj *
parse(const vector<string>& tokens) {
  auto ret = parse_impl(tokens, 0, 1).first;
  // display(*ret);
  return ret;
}

}
