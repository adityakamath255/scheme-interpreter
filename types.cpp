#include "types.hpp"
#include <memory>

namespace Scheme {

using LambdaBody = std::shared_ptr<Expression>;

bool is_bool(const Obj& obj) {return std::holds_alternative<bool>(obj); }
bool is_true(const Obj& obj) {return (!is_bool(obj) || as_bool(obj) == true); }
bool is_false(const Obj& obj) {return !is_true(obj); }
bool is_number(const Obj& obj) {return std::holds_alternative<double>(obj); }
bool is_symbol(const Obj& obj){return std::holds_alternative<Symbol>(obj); }
bool is_string(const Obj& obj) {return std::holds_alternative<std::string>(obj); }
bool is_pair(const Obj& obj) {return std::holds_alternative<Cons*>(obj); }
bool is_primitive(const Obj& obj) {return std::holds_alternative<Primitive*>(obj); }
bool is_procedure(const Obj& obj) {return std::holds_alternative<Procedure*>(obj); }
bool is_callable(const Obj& obj) {return is_primitive(obj) || is_procedure(obj); }
bool is_null(const Obj& obj) {return std::holds_alternative<nullptr_t>(obj); }
bool is_void(const Obj& obj) {return std::holds_alternative<Void>(obj); }

bool& as_bool(Obj& obj) { return std::get<bool>(obj); }
const bool& as_bool(const Obj& obj) { return std::get<bool>(obj); }

double& as_number(Obj& obj) { return std::get<double>(obj); }
const double& as_number(const Obj& obj) { return std::get<double>(obj); }

Symbol& as_symbol(Obj& obj) { return std::get<Symbol>(obj); }
const Symbol& as_symbol(const Obj& obj) { return std::get<Symbol>(obj); }

std::string& as_string(Obj& obj) { return std::get<std::string>(obj); }
const std::string& as_string(const Obj& obj) { return std::get<std::string>(obj); }

Cons*& as_pair(Obj& obj) { return std::get<Cons*>(obj); }
Cons* const& as_pair(const Obj& obj) { return std::get<Cons*>(obj); }

Primitive*& as_primitive(Obj& obj) { return std::get<Primitive*>(obj); }
Primitive* const& as_primitive(const Obj& obj) { return std::get<Primitive*>(obj); }

Procedure*& as_procedure(Obj& obj) { return std::get<Procedure*>(obj); }
Procedure* const& as_procedure(const Obj& obj) { return std::get<Procedure*>(obj); }

std::nullptr_t& as_null(Obj& obj) { return std::get<std::nullptr_t>(obj); }
const std::nullptr_t& as_null(const Obj& obj) { return std::get<std::nullptr_t>(obj); }

Void& as_void(Obj& obj) { return std::get<Void>(obj); }
const Void& as_void(const Obj& obj) { return std::get<Void>(obj); }

std::unordered_map<std::string, std::string*> Symbol::intern_table;

Symbol::Symbol(const std::string& s) {
  auto itr = intern_table.find(s);
  if (itr == intern_table.end()) {
    auto new_str = new std::string(s);
    intern_table[s] = new_str;
    id = new_str;
  }
  else {
    id = itr->second;
  }
}

Symbol::Symbol(): id {nullptr} {}

const std::string&
Symbol::get_name() const {
  return *id;
}

bool
Symbol::operator ==(const Symbol& other) const {
  return id == other.id;
}

Cons::Cons(Obj a, Obj b): car {std::move(a)}, cdr {std::move(b)} {}

Obj 
Cons::at(const std::string& s) {
  Obj curr = const_cast<Cons*>(this);

  if (*s.begin() != 'c' || *s.rbegin() != 'r') {
    throw std::runtime_error("invalid cons operation: " + s);
  }

  for (int i = s.size() - 2; i > 0; i--) {
    switch (s[i]) {
      case 'a':
        curr = as_pair(curr)->car;
        break;
      case 'd':
        curr = as_pair(curr)->cdr;
        break;
      default:
        throw std::runtime_error("invalid cons operation: " + s);
    }
  }

  return curr;
}

Primitive::Primitive(decltype(func) f): 
  func {f} 
{}

Obj 
Primitive::operator ()(const ArgList& args) const {
  return func(args);
}

Procedure::Procedure(ParamList p, LambdaBody b, Environment *e):
  parameters {p},
  body {b},
  env {e}
{}

bool operator ==(const Void& v0, const Void& v1) {return true; }

}

