#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <variant>
#include <algorithm>
#include <functional>

using std::cin;
using std::cout;
using std::cerr;
using std::vector;
using std::string;
using std::runtime_error;

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

bool& as_bool(Obj& obj);
const bool& as_bool(const Obj& obj);

double& as_number(Obj& obj);
const double& as_number(const Obj& obj);

Symbol& as_symbol(Obj& obj);
const Symbol& as_symbol(const Obj& obj);

std::string& as_string(Obj& obj);
const std::string& as_string(const Obj& obj);

Cons*& as_pair(Obj& obj);
Cons* const& as_pair(const Obj& obj);

Primitive*& as_primitive(Obj& obj);
Primitive* const& as_primitive(const Obj& obj);

Procedure*& as_procedure(Obj& obj);
Procedure* const& as_procedure(const Obj& obj);

std::nullptr_t& as_null(Obj& obj);
const std::nullptr_t& as_null(const Obj& obj);

Void& as_void(Obj& obj);
const Void& as_void(const Obj& obj);

struct Symbol {
  string name;
  Symbol();
  Symbol(const string& s);
};

bool operator ==(const Symbol&, const Symbol&);
bool operator <(const Symbol&, const Symbol&);

bool operator ==(const Void, const Void);

struct Cons {
  Obj car;
  Obj cdr;
  Cons(Obj a, Obj b);
  Obj at(const string& s);
};

struct Primitive {
  Obj(*func)(const vector<Obj>&);
  Primitive(decltype(func) f);
  Obj operator()(const vector<Obj>& args) const;
};

struct Procedure {
  const vector<Symbol> parameters;
  const Expression *body;
  Environment *const env;
  Procedure(vector<Symbol> p, Expression *b, Environment *e);
};

}
