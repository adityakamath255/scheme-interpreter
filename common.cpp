#include "common.hpp"

namespace Scheme {

bool is_bool(const Obj& obj) {return std::holds_alternative<bool>(obj); }
bool is_true(const Obj& obj) {return (!is_bool(obj) || get<bool>(obj) == true); }
bool is_false(const Obj& obj) {return !is_true(obj); }
bool is_number(const Obj& obj) {return std::holds_alternative<double>(obj); }
bool is_symbol(const Obj& obj){return std::holds_alternative<Symbol>(obj); }
bool is_string(const Obj& obj) {return std::holds_alternative<string>(obj); }
bool is_pair(const Obj& obj) {return std::holds_alternative<Cons*>(obj); }
bool is_primitive(const Obj& obj) {return std::holds_alternative<Primitive*>(obj); }
bool is_procedure(const Obj& obj) {return std::holds_alternative<Procedure*>(obj); }
bool is_callable(const Obj& obj) {return is_primitive(obj) || is_procedure(obj); }
bool is_null(const Obj& obj) {return std::holds_alternative<nullptr_t>(obj); }
bool is_void(const Obj& obj) {return std::holds_alternative<Void>(obj); }

Symbol::Symbol(): name {"*undefined*"} {}
Symbol::Symbol(const string& s): name {s} {}

bool operator
==(const Symbol& s0, const Symbol& s1) {
  return s0.name == s1.name;
}

bool operator 
<(const Symbol& s0, const Symbol& s1) {
  return s0.name < s1.name;
}

bool operator
==(const Void v0, const Void v1) {
  return true;
}

Cons::Cons(Obj a, Obj b): car {a}, cdr {b} {}

Obj 
Cons::at(const string& s) {
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

Primitive::Primitive(decltype(func) f): 
  func {f} 
{}

Obj 
Primitive::operator ()(const vector<Obj>& args) const {
  return func(args);
}

Procedure::Procedure(vector<Symbol> p, Expression *b, Environment *e):
  parameters {p},
  body {b},
  env {e}
{}

std::map<Symbol, Obj>::iterator
Environment::assoc(const Symbol& s) {
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

Environment::Environment():
  super {nullptr} {}
  
Environment::Environment(Environment *super_): 
  super {super_} {}

void
Environment::set_variable(const Symbol& s, const Obj obj) {
  assoc(s)->second = obj;
}

Obj 
Environment::lookup(const Symbol& s) {
  return assoc(s)->second;
}

void
Environment::define_variable(const Symbol& s, Obj obj) {
  const auto found = frame.find(s);
  if (found != frame.end()) {
    throw runtime_error("binding already present: " + s.name);
  }
  else {
    frame.insert({s, obj});
  }
}

Environment *
Environment::extend(const vector<Symbol>& parameters, const vector<Obj>& arguments) {
  if (parameters.size() != arguments.size()) {
    throw runtime_error("env extend size mismatch");
  }
  Environment *ret = new Environment(this);
  for (int i = 0; i < parameters.size(); i++) {
    ret->define_variable(parameters[i], arguments[i]);
  }
  return ret;
}

TailCall::TailCall(Obj proc, vector<Obj> args): 
  proc {proc}, 
  args {move(args)} 
{}

int
Expression::get_size(Cons* obj) const {
  int sz = 0;
  while (obj != nullptr) {
    sz++;
    if (!is_pair(obj->cdr)) {
      break;
    }
    else {
      obj = get<Cons*>(obj->cdr);
    }
  }
  return sz;
}

void
Expression::assert_size(Cons *obj, const int lb, const int ub) const {
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

Expression::Expression(const string& n, Cons *obj, const int lb, const int ub):
  name {n} 
{
  if (lb != -1) {
    assert_size(obj, lb, ub);
  }
}

string
Expression::get_name() const {
  return name;
}

EvalResult 
eval(Expression *expr, Environment *const env) {
  return expr->eval(env);
}

EvalResult 
apply(Obj p, vector<Obj> args) {
  while (true) {
    if (is_primitive(p)) {
      const auto func = *get<Primitive*>(p);
      return func(args);
    }
    else if (is_procedure(p)) {
      const auto func = get<Procedure*>(p);
      if (func->parameters.size() != args.size()) {
        throw runtime_error(" wrong number of arguments: expected " + std::to_string(func->parameters.size()));
      }
      const auto new_env = func->env->extend(func->parameters, args);
      auto res = func->body->eval(new_env);
      if (std::holds_alternative<Obj>(res)) {
        return get<Obj>(res);
      }
      else if (std::holds_alternative<TailCall>(res)) {
        p = get<TailCall>(res).proc;
        args = get<TailCall>(res).args;
      }
    }
    else {
      throw runtime_error("tried to apply an object that is not a procedure");
    }
  } 
}

}

