#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <variant>
#include <stack>

namespace Scheme { 

class Symbol;
class Cons;
class Vector;
class Primitive;
class Procedure;
class Void {};

class Environment;
class Expression;
class Interpreter;

using Obj = std::variant<
  bool,
  double,
  Symbol,
  std::string,
  Cons*,
  Vector*,
  Primitive*,
  Procedure*,
  nullptr_t,
  Void
>;

using ParamList = std::vector<Symbol>;
using ArgList = std::vector<Obj>;

class HeapEntity {
public:
  bool marked;
  HeapEntity(): marked {false} {}
  virtual void push_children(std::stack<HeapEntity*>&) {};
  virtual ~HeapEntity() = default;
};

class Symbol { 
  friend struct std::hash<Symbol>;

private:
  const std::string *id;

public:
  Symbol(): id {nullptr} {}
  Symbol(const std::string* id): id {id} {};
  const std::string& get_name() const {
    return *id; 
  }
  bool operator ==(const Symbol& other) const {
    return id == other.id; 
  }
};

class Cons : public HeapEntity {
private:
  void push_children(std::stack<HeapEntity*>&) override;
public:
  Obj car;
  Obj cdr;
  Cons(Obj car, Obj cdr):
    car {std::move(car)},
    cdr {std::move(cdr)}
  {} 
  Obj at(const std::string&);
};

class Vector : public HeapEntity {
private:
  void push_children(std::stack<HeapEntity*>&) override;
public:
  std::vector<Obj> data;
  Vector(std::vector<Obj> data): data {std::move(data)} {}
};

class Primitive : public HeapEntity {
private:
  Obj(*func)(const ArgList&, Interpreter&);
  void push_children(std::stack<HeapEntity*>&) override;
public:
  Primitive(decltype(func) f): func {f} {};
  Obj operator()(const ArgList& args, Interpreter& interp) const {
    return func(args, interp);
  }
};

class Procedure : public HeapEntity {
private:
  void push_children(std::stack<HeapEntity*>&) override;
public:
  const ParamList parameters;
  Expression *body;
  Environment *const env;
  bool is_variadic;
  Procedure(ParamList p, Expression *b, Environment* e, bool v):
    parameters {std::move(p)},
    body {b},
    env {e},
    is_variadic {v}
  {}
};

inline bool is_bool(const Obj& obj) {return std::holds_alternative<bool>(obj);}
inline bool is_number(const Obj& obj) {return std::holds_alternative<double>(obj);}
inline bool is_symbol(const Obj& obj){return std::holds_alternative<Symbol>(obj);}
inline bool is_string(const Obj& obj) {return std::holds_alternative<std::string>(obj);}
inline bool is_pair(const Obj& obj) {return std::holds_alternative<Cons*>(obj);}
inline bool is_vector(const Obj& obj) {return std::holds_alternative<Vector*>(obj);}
inline bool is_primitive(const Obj& obj) {return std::holds_alternative<Primitive*>(obj);}
inline bool is_procedure(const Obj& obj) {return std::holds_alternative<Procedure*>(obj);}
inline bool is_callable(const Obj& obj) {return is_primitive(obj) || is_procedure(obj);}
inline bool is_null(const Obj& obj) {return std::holds_alternative<nullptr_t>(obj);}
inline bool is_void(const Obj& obj) {return std::holds_alternative<Void>(obj);}

inline bool& as_bool(Obj& obj) {return std::get<bool>(obj);}
inline const bool& as_bool(const Obj& obj) {return std::get<bool>(obj);}

inline double& as_number(Obj& obj) {return std::get<double>(obj);}
inline const double& as_number(const Obj& obj) {return std::get<double>(obj);}

inline Symbol& as_symbol(Obj& obj) {return std::get<Symbol>(obj);}
inline const Symbol& as_symbol(const Obj& obj) {return std::get<Symbol>(obj);}

inline std::string& as_string(Obj& obj) {return std::get<std::string>(obj);}
inline const std::string& as_string(const Obj& obj) {return std::get<std::string>(obj);}

inline Cons*& as_pair(Obj& obj) {return std::get<Cons*>(obj);}
inline Cons* const& as_pair(const Obj& obj) {return std::get<Cons*>(obj);}

inline Vector*& as_vector(Obj& obj) {return std::get<Vector*>(obj);}
inline Vector* const& as_vector(const Obj& obj) {return std::get<Vector*>(obj);}

inline Primitive*& as_primitive(Obj& obj) {return std::get<Primitive*>(obj);}
inline Primitive* const& as_primitive(const Obj& obj) {return std::get<Primitive*>(obj);}

inline Procedure*& as_procedure(Obj& obj) {return std::get<Procedure*>(obj);}
inline Procedure* const& as_procedure(const Obj& obj) {return std::get<Procedure*>(obj);}

inline std::nullptr_t& as_null(Obj& obj) {return std::get<std::nullptr_t>(obj);}
inline const std::nullptr_t& as_null(const Obj& obj) {return std::get<std::nullptr_t>(obj);}

inline Void& as_void(Obj& obj) {return std::get<Void>(obj);}
inline const Void& as_void(const Obj& obj) {return std::get<Void>(obj);}

inline bool is_true(const Obj& obj) {return (!is_bool(obj) || as_bool(obj) == true);}
inline bool is_false(const Obj& obj) {return !is_true(obj);}

inline bool operator ==(const Void& v0, const Void& v1) {return true;}

std::string stringify(const Obj&);

}

namespace std {
  template<>
  struct hash<Scheme::Symbol> {
    size_t operator()(const Scheme::Symbol& s) const {
      return hash<const void*>()(s.id);
    }
  };
}
