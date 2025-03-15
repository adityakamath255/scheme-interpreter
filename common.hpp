#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include <algorithm>
#include <cmath>
#include <string>
#include <functional>

using namespace std;

namespace scheme {

class symbol;
class cons;
class primitive;
class procedure;
class environment;
class executor;
class expression;

typedef variant<
  bool,
  double,
  symbol,
  string,
  cons*,
  primitive*,
  procedure*,
  nullptr_t
> sc_obj;

struct symbol {
  string name;

  symbol(): name {"*undefined*"} {}

  symbol(string s): name {s} {}

  friend bool
  operator==(const symbol& s0, const symbol& s1) {
    return s0.name == s1.name;
  }

  friend bool
  operator<(const symbol& s0, const symbol& s1) {
    return s0.name < s1.name;
  }
};

struct cons {
  sc_obj car;
  sc_obj cdr;
  cons(sc_obj a, sc_obj b): car {a}, cdr {b} {}

  sc_obj
  operator[](const string& s) {
    sc_obj curr = const_cast<cons*>(this);

    if (*s.begin() != 'c' || *s.rbegin() != 'r') {
      throw runtime_error("invalid cons operation: " + s);
      return nullptr;
    }

    for (int i = s.size() - 2; i > 0; i--) {
      switch (s[i]) {
        case 'a':
          curr = get<cons*>(curr)->car;
          break;
        case 'd':
          curr = get<cons*>(curr)->cdr;
          break;
        default:
          throw runtime_error("invalid cons operation: " + s);
          return nullptr;
      }
    }

    return curr;
  }

  sc_obj
  at(const string& s) {
    return operator[](s);
  } 
};

struct primitive {
  function<sc_obj(const vector<sc_obj>&)> func;
  primitive(decltype(func) f): func {f} {}

  sc_obj 
  operator()(const vector<sc_obj>& args) const {
    return func(args);
  }
};

struct procedure {
  const vector<symbol> parameters;
  const executor *body;
  environment *const env;

  procedure(vector<symbol> p, executor *b, environment *e):
    parameters {p},
    body {b},
    env {e}
  {}
};

class environment {
private:
  map<symbol, sc_obj> frame {};
  environment *const super;

  map<symbol, sc_obj>::iterator
  assoc(const symbol& s) {
    const auto found = frame.find(s);
    if (found != frame.end()) {
      return found;
    }
    else if (super != nullptr) {
      return super->assoc(s);
    }
    else {
      throw runtime_error("Unbound variable: " + s.name);
    }
    return found;
  }

public:
  environment():
    super {nullptr} {}
    
  environment(environment *super_): 
    super {super_} {}

  void
  set_variable(const symbol& s, const sc_obj obj) {
    assoc(s)->second = obj;
  }

  sc_obj 
  lookup(const symbol& s) {
    return assoc(s)->second;
  }

  void
  define_variable(const symbol& s, sc_obj obj) {
    const auto found = frame.find(s);
    if (found != frame.end()) {
      throw runtime_error("binding already present: " + s.name);
    }
    else {
      frame.insert({s, obj});
    }
  }
};

environment* 
extend_env(const vector<symbol>& parameters, const vector<sc_obj>& arguments, environment *env) {
  if (parameters.size() != arguments.size()) {
    throw runtime_error("env extend size mismatch");
    return nullptr; 
  }
  environment *ret = new environment(env);
  for (int i = 0; i < parameters.size(); i++) {
    ret->define_variable(parameters[i], arguments[i]);
  }
  return ret;
}

class executor {
public:
  virtual sc_obj
  eval(environment*) const = 0;
};

class expression {
private:
  int 
  get_size(cons* obj) const {
    int sz = 0;
    while (obj != nullptr) {
      sz++;
      if (!holds_alternative<cons*>(obj->cdr)) {
        break;
      }
      else {
        obj = get<cons*>(obj->cdr);
      }
    }
    return sz;
  }

  void
  assert_size(cons *obj, const int lb, const int ub) const {
    const int sz = get_size(obj);
    if (sz < lb || sz > ub) {
      throw runtime_error(
        name + 
        " expression is of wrong size [" + 
        to_string(sz) + 
        "]"
      );
    }
  }

protected:
  string name;

public:
  expression(const string& n, cons *obj = nullptr, const int lb = -1, const int ub = -1):
    name {n} 
  {
    if (lb != -1) {
      assert_size(obj, lb, ub);
    }
  }

  string
  get_name() const {
    return name;
  }

  virtual executor*
  analyze() const = 0;

  virtual sc_obj
  eval(environment*) const {}; 
};

bool
is_pair(const sc_obj obj) {
  return holds_alternative<cons*>(obj);
}

bool 
is_null(const sc_obj obj) {
  return holds_alternative<nullptr_t>(obj);
}

bool 
is_true(const sc_obj obj) {
  return (
    !holds_alternative<bool>(obj) ||
    get<bool>(obj) == true
  );
}

bool
is_false(const sc_obj obj) {
  return !is_true(obj);
}

expression*
classify(const sc_obj);

sc_obj 
eval(expression *expr, environment *const env) {
  return expr->eval(env);
}

sc_obj 
apply(const sc_obj p, const vector<sc_obj>& args) {
  if (holds_alternative<primitive*>(p)) {
    const auto func = *get<primitive*>(p);
    return func(args);
  }
  else if (holds_alternative<procedure*>(p)) {
    const auto pp = get<procedure*>(p);
    const auto new_env = extend_env(pp->parameters, args, pp->env);
    return pp->body->eval(new_env);
  }
  else {
    throw runtime_error("tried to apply an object that is not a procedure");
    return nullptr;
  }
}

vector<symbol>
cons2symbols(sc_obj c) {
  vector<symbol> ret;
  while (is_pair(c)) {
    const auto as_cons = get<cons*>(c);
    ret.push_back(get<symbol>(as_cons->car));
    c = as_cons->cdr;
  }
  return ret;
}

vector<expression*> 
cons2vec(sc_obj seq) {
  vector<expression*> vec {};
  while (is_pair(seq)) {
    const auto as_cons = get<cons*>(seq);
    vec.push_back(classify(as_cons->car));
    seq = as_cons->cdr; 
  }
  return vec;
}

vector<executor*> 
exprs2execs(const vector<expression*>& arr) {
  vector<executor*> ret {};
  for (const auto expr : arr)
    ret.push_back(expr->analyze());
  return ret;
}

constexpr double precision = 1e-9;

}
