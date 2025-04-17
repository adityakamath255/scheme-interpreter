#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <variant>

using std::vector;
using std::string;

namespace Scheme {

class Symbol;
class Cons;
class Primitive;
class Procedure;
class Environment;
class Expression;
class Void {};

using Obj = std::variant<
  bool,
  double,
  Symbol,
  string,
  Cons*,
  Primitive*,
  Procedure*,
  nullptr_t,
  Void
>;

bool is_bool(const Obj&);
bool is_true(const Obj&);
bool is_false(const Obj&);
bool is_number(const Obj&);
bool is_symbol(const Obj&);
bool is_string(const Obj&);
bool is_pair(const Obj&);
bool is_primitive(const Obj&);
bool is_procedure(const Obj&);
bool is_callable(const Obj&);
bool is_null(const Obj&);
bool is_void(const Obj&);

bool& as_bool(Obj&);
const bool& as_bool(const Obj&);

double& as_number(Obj&);
const double& as_number(const Obj&);

Symbol& as_symbol(Obj&);
const Symbol& as_symbol(const Obj&);

string& as_string(Obj&);
const string& as_string(const Obj&);

Cons*& as_pair(Obj&);
Cons* const& as_pair(const Obj&);

Primitive*& as_primitive(Obj&);
Primitive* const& as_primitive(const Obj&);

Procedure*& as_procedure(Obj&);
Procedure* const& as_procedure(const Obj&);

std::nullptr_t& as_null(Obj&);
const std::nullptr_t& as_null(const Obj&);

Void& as_void(Obj&);
const Void& as_void(const Obj&);

struct Symbol {
  string name;
  Symbol();
  Symbol(const string&);
};

bool operator ==(const Symbol&, const Symbol&);
bool operator <(const Symbol&, const Symbol&);

bool operator ==(const Void, const Void);

struct Cons {
  Obj car;
  Obj cdr;
  Cons(Obj, Obj);
  Obj at(const string&);
};

struct Primitive {
  Obj(*func)(const vector<Obj>&);
  Primitive(decltype(func) f);
  Obj operator()(const vector<Obj>&) const;
};

struct Procedure {
  const vector<Symbol> parameters;
  const Expression *body;
  Environment *const env;
  Procedure(vector<Symbol>, Expression*, Environment*);
};

}
