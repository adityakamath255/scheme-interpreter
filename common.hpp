#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <variant>
#include <algorithm>
#include <functional>

using std::cout;
using std::vector;
using std::string;
using std::get;
using std::holds_alternative;
using std::runtime_error;

namespace Scheme {

class Symbol;
class Cons;
class Primitive;
class Procedure;
class Environment;
class Expression;

typedef std::variant<
  bool,
  double,
  Symbol,
  string,
  Cons*,
  Primitive*,
  Procedure*,
  nullptr_t
> Obj;

struct Symbol {
  string name;

  Symbol(): name {"*undefined*"} {}

  Symbol(const string& s): name {s} {}

  friend bool
  operator==(const Symbol& s0, const Symbol& s1) {
    return s0.name == s1.name;
  }

  friend bool
  operator<(const Symbol& s0, const Symbol& s1) {
    return s0.name < s1.name;
  }
};

struct Cons {
  Obj car;
  Obj cdr;
  Cons(Obj a, Obj b): car {a}, cdr {b} {}

  Obj
  operator[](const string& s) {
    Obj curr = const_cast<Cons*>(this);

    if (*s.begin() != 'c' || *s.rbegin() != 'r') {
      throw runtime_error("invalid cons operation: " + s);
    }

    for (int i = s.size() - 2; i > 0; i--) {
      switch (s[i]) {
        case 'a':
          curr = get<Cons*>(curr)->car;
          break;
        case 'd':
          curr = get<Cons*>(curr)->cdr;
          break;
        default:
          throw runtime_error("invalid cons operation: " + s);
      }
    }

    return curr;
  }

  Obj
  at(const string& s) {
    return operator[](s);
  } 
};

struct Primitive {
  std::function<Obj(const vector<Obj>&)> func;
  Primitive(decltype(func) f): func {f} {}

  Obj 
  operator()(const vector<Obj>& args) const {
    return func(args);
  }
};

struct Procedure {
  const vector<Symbol> parameters;
  const Expression *body;
  Environment *const env;

  Procedure(vector<Symbol> p, Expression *b, Environment *e):
    parameters {p},
    body {b},
    env {e}
  {}
};

class Environment {
private:
  std::map<Symbol, Obj> frame {};
  Environment *const super;

  decltype(frame)::iterator
  assoc(const Symbol& s) {
    const auto found = frame.find(s);
    if (found != frame.end()) {
      return found;
    }
    else if (super != nullptr) {
      return super->assoc(s);
    }
    else {
      throw runtime_error("unbound variable: " + s.name);
    }
    return found;
  }

public:
  Environment():
    super {nullptr} {}
    
  Environment(Environment *super_): 
    super {super_} {}

  void
  set_variable(const Symbol& s, const Obj obj) {
    assoc(s)->second = obj;
  }

  Obj 
  lookup(const Symbol& s) {
    return assoc(s)->second;
  }

  void
  define_variable(const Symbol& s, Obj obj) {
    const auto found = frame.find(s);
    if (found != frame.end()) {
      throw runtime_error("binding already present: " + s.name);
    }
    else {
      frame.insert({s, obj});
    }
  }
  
  Environment *
  extend(const vector<Symbol>& parameters, const vector<Obj>& arguments) {
    if (parameters.size() != arguments.size()) {
      throw runtime_error("env extend size mismatch");
    }
    Environment *ret = new Environment(this);
    for (int i = 0; i < parameters.size(); i++) {
      ret->define_variable(parameters[i], arguments[i]);
    }
    return ret;
  }
};

struct TailCall {
  Obj proc;
  vector<Obj> args;

  TailCall(Obj& proc, vector<Obj>& args): proc {proc}, args {args} {}
};  

class Expression {
private:
  int 
  get_size(Cons* obj) const {
    int sz = 0;
    while (obj != nullptr) {
      sz++;
      if (!holds_alternative<Cons*>(obj->cdr)) {
        break;
      }
      else {
        obj = get<Cons*>(obj->cdr);
      }
    }
    return sz;
  }

  void
  assert_size(Cons *obj, const int lb, const int ub) const {
    const int sz = get_size(obj);
    if (sz < lb || sz > ub) {
      throw runtime_error(
        name + 
        " expression is of wrong size [" + 
        std::to_string(sz) + 
        "]"
      );
    }
  }

protected:
  string name;

public:
  Expression(const string& n, Cons *obj = nullptr, const int lb = -1, const int ub = -1):
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

  virtual Obj
  eval(Environment*) const = 0; 

  virtual void
  tco() {}
};

bool
is_pair(const Obj obj) {
  return holds_alternative<Cons*>(obj);
}

bool 
is_null(const Obj obj) {
  return holds_alternative<nullptr_t>(obj);
}

bool 
is_true(const Obj obj) {
  return (
    !holds_alternative<bool>(obj) ||
    get<bool>(obj) == true
  );
}

bool
is_false(const Obj obj) {
  return !is_true(obj);
}

Expression*
classify(const Obj);

Obj 
eval(Expression *expr, Environment *const env) {
  return expr->eval(env);
}

Obj 
apply(Obj p, vector<Obj> args) {
  while (true) {
    try {
      if (holds_alternative<Primitive*>(p)) {
        const auto func = *get<Primitive*>(p);
        return func(args);
      }
      else if (holds_alternative<Procedure*>(p)) {
        const auto func = get<Procedure*>(p);
        if (func->parameters.size() != args.size()) {
          throw runtime_error(" wrong number of arguments: expected " + std::to_string(func->parameters.size()));
        }
        const auto new_env = func->env->extend(func->parameters, args);
        return func->body->eval(new_env);
      }
      else {
        throw runtime_error("tried to apply an object that is not a procedure");
      }
    }
    catch (TailCall tc) {
      p = tc.proc;
      args = tc.args;
    }
  } 
}

vector<Symbol>
cons2symbols(Obj c) {
  vector<Symbol> vec {};
  while (is_pair(c)) {
    const auto as_cons = get<Cons*>(c);
    vec.push_back(get<Symbol>(as_cons->car));
    c = as_cons->cdr;
  }
  return vec;
}

vector<Expression*> 
cons2vec(Obj seq) {
  vector<Expression*> vec {};
  while (is_pair(seq)) {
    const auto as_cons = get<Cons*>(seq);
    vec.push_back(classify(as_cons->car));
    seq = as_cons->cdr; 
  }
  return vec;
}

constexpr double precision = 1e-9;

}
