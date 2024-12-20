#include "interface.hpp"

namespace scheme {

vector<string>
tokenize(const string& input) {
  const int sz = input.size();
  const vector<char> specials {'(', ')', '\'', '`', ',', '\"', ';'};
  vector<string> tokens {"(", "begin"};

  let insert_and_clear = [&](string *token) {
    if (!token->empty()) {
      tokens.push_back(*token);
      token->clear();
    }
  };

  string token {};
  bool is_string = 0;

  for (int i = 0; i < sz; i++) {
    char c = input[i];

    if (c == '"') {
      insert_and_clear(&token);
      insert_and_clear(new string("\""));
      is_string = !is_string;
    }
    else if (!is_string) {
      if (isspace(c)) {
        insert_and_clear(&token);
      }

      else if (find(specials.begin(), specials.end(), c) != specials.end()) {
        insert_and_clear(&token);
        insert_and_clear(new string(1, c));
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
parse_impl(const vector<string>& tokens, const int i, bool recursive) {
  const string token = tokens[i];
  sc_obj *head;
  int j;

  {
    if (token[0] == ')')  {
      return {new sc_obj(nullptr), i + 1};
    }
    
    else if (token[0] == '(') {
      tie(head, j) = parse_impl(tokens, i + 1, 1);
    }

    else if (token[0] == '.') {
      return {sc_obj_from_str(tokens[i + 1]), i + 3};
    }

    else if (token[0] == '\'') {
      sc_obj *q;
      tie(q, j) = parse_impl(tokens, i + 1, 0);
      head = new sc_obj(
        new cons(
          *make_sym_obj("quote"), 
          new cons(
            *q, 
            nullptr)));
    }

    else {
      tie(head, j) = make_pair(sc_obj_from_str(token), i + 1);
    }
  } 

  {
    if (j == tokens.size() || !recursive) {
      return {head, j};
    }

    else {
      if (recursive) {
        let [tail, k] = parse_impl(tokens, j, 1);
        cons *c = new cons(*head, *tail);
        return {new sc_obj(c), k};
      }
    }
  }
}

sc_obj *
parse(const vector<string>& tokens) {
  return parse_impl(tokens, 0, 0).first;
}

}