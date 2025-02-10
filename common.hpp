#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include <algorithm>
#include <cmath>
#include <string>
#include <functional>
#include <memory>

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
  shared_ptr<cons>,
  shared_ptr<primitive>,
  shared_ptr<procedure>,
  nullptr_t
> sc_obj;

void display(const sc_obj);

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

struct cons : public enable_shared_from_this<cons> {
  sc_obj car;
  sc_obj cdr;
  cons(sc_obj a, sc_obj b): car {a}, cdr {b} {}

  sc_obj
  operator[](const string& s) {
    if (*s.begin() != 'c' || *s.rbegin() != 'r') {
      throw runtime_error("invalid cons operation: " + s);
    }

    sc_obj curr = shared_from_this();

    for (int i = s.size() - 2; i > 0; i--) {
      switch (s[i]) {
        case 'a':
          curr = get<shared_ptr<cons>>(curr)->car;
          break;
        case 'd':
          curr = get<shared_ptr<cons>>(curr)->cdr;
          break;
        default:
          throw runtime_error("invalid cons operation: " + s);
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
  const shared_ptr<executor> body;
  shared_ptr<environment> env;

  procedure(vector<symbol> p, shared_ptr<executor> b, shared_ptr<environment> e):
    parameters {p},
    body {b},
    env {e}
  {}
};

class environment {
private:
  map<symbol, sc_obj> frame {};
  shared_ptr<environment> super;

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
    
  environment(shared_ptr<environment> super_): 
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

  void debug_frame() const {
        for (const auto& [key, value] : frame) {
            display(value);
        }
    }
};

shared_ptr<environment>
extend_env(const vector<symbol>& parameters, const vector<sc_obj>& arguments, shared_ptr<environment> env) {
  if (parameters.size() != arguments.size()) {
    throw runtime_error("env extend size mismatch");
    return nullptr; 
  }
  auto ret = make_shared<environment>(env);
  for (int i = 0; i < parameters.size(); i++) {
    ret->define_variable(parameters[i], arguments[i]);
  }
  return ret;
}

class executor {
public:
  virtual sc_obj
  eval(shared_ptr<environment>) const = 0;
};

class expression {
private:
  int 
  get_size(shared_ptr<cons> obj) const {
    int sz = 0;
    while (obj != nullptr) {
      sz++;
      if (!holds_alternative<shared_ptr<cons>>(obj->cdr)) {
        break;
      }
      else {
        obj = get<shared_ptr<cons>>(obj->cdr);
      }
    }
    return sz;
  }

  void
  assert_size(shared_ptr<cons>obj, const int lb, const int ub) const {
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
  expression(const string& n, shared_ptr<cons>obj = nullptr, const int lb = -1, const int ub = -1):
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

  virtual shared_ptr<executor> 
  analyze() const = 0;
};

bool
is_pair(const sc_obj obj) {
  return holds_alternative<shared_ptr<cons>>(obj);
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

shared_ptr<expression>
classify(const sc_obj);

sc_obj 
eval(shared_ptr<expression> expr, shared_ptr<environment> env) {
    if (!expr) {
        throw runtime_error("Null expression in eval");
    }
    
    auto executor = expr->analyze();
    if (!executor) {
        throw runtime_error("Null executor from analysis");
    }
    
    return executor->eval(env);
}

sc_obj 
apply(const sc_obj p, const vector<sc_obj>& args) {
  if (holds_alternative<shared_ptr<primitive>>(p)) {
    const auto func = *get<shared_ptr<primitive>>(p);
    return func(args);
  }
  else if (holds_alternative<shared_ptr<procedure>>(p)) {
    const auto pp = get<shared_ptr<procedure>>(p);
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
    const auto as_cons = get<shared_ptr<cons>>(c);
    ret.push_back(get<symbol>(as_cons->car));
    c = as_cons->cdr;
  }
  return ret;
}

vector<shared_ptr<expression>> 
cons2vec(sc_obj seq) {
  vector<shared_ptr<expression>> vec {};
  while (is_pair(seq)) {
    const auto as_cons = get<shared_ptr<cons>>(seq);
    vec.push_back(classify(as_cons->car));
    seq = as_cons->cdr; 
  }
  return vec;
}

vector<shared_ptr<executor>> 
exprs2execs(const vector<shared_ptr<expression>>& arr) {
  vector<shared_ptr<executor>> ret {};
  for (const auto expr : arr)
    ret.push_back(expr->analyze());
  return ret;
}


constexpr double precision = 1e-7;

}
