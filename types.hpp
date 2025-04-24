#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <variant>
#include <unordered_map>
#include <memory>

namespace Scheme {

class Symbol;
class Cons;
class Primitive;
class Procedure;
class Void {};

class Environment;
class Expression;

using Obj = std::variant<
  bool,
  double,
  Symbol,
  std::string,
  Cons*,
  Primitive*,
  Procedure*,
  nullptr_t,
  Void
>;

using ParamList = std::vector<Symbol>;
using ArgList = std::vector<Obj>;
using LambdaBody = std::shared_ptr<Expression>;

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

std::string& as_string(Obj&);
const std::string& as_string(const Obj&);

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
  friend struct std::hash<Symbol>;

private:
  const std::string *id;

public:
  Symbol();
  Symbol(const std::string*);
  const std::string& get_name() const;
  bool operator ==(const Symbol& other) const;
};

class Cons {
public:
  Obj car;
  Obj cdr;
  Cons(Obj, Obj);
  Obj at(const std::string&);
};

class Primitive {
private:
  Obj(*func)(const ArgList&);
public:
  Primitive(decltype(func) f);
  Obj operator()(const ArgList&) const;
};

class Procedure {
public:
  const ParamList parameters;
  const LambdaBody body;
  Environment *const env;
  Procedure(ParamList, LambdaBody, Environment*);
};

bool 
operator ==(const Void& v0, const Void& v1);

}

namespace std {
  template<>
  struct hash<Scheme::Symbol> {
    size_t operator()(const Scheme::Symbol& s) const {
      return hash<const void*>()(s.id);
    }
  };
}
