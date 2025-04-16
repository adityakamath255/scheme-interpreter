#include "types.hpp"

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

Symbol as_symbol(Obj& obj) {return std::get<Symbol>(obj); }

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

Cons::Cons(Obj a, Obj b): car {std::move(a)}, cdr {std::move(b)} {}

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

}

