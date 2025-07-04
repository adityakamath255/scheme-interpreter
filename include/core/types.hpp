#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <variant>
#include <stack>
#include <functional>

namespace Scheme { 

class Symbol;
class String;
class Cons;
class Vector;
class Builtin;
class Procedure;
class Null {};
class Void {};

class Environment;
class Expression;
class Interpreter;

using Obj = std::variant<
  bool,
  double,
  char,
  Symbol,
  String*,
  Cons*,
  Vector*,
  Builtin*,
  Procedure*,
  Null,
  Void
>;

using ParamList = std::vector<Symbol>;
using ArgList = std::vector<Obj>;

class HeapEntity;
using MarkStack = std::stack<HeapEntity*>;

class HeapEntity {
public:
  bool marked;
  HeapEntity(): marked {false} {}
  virtual void push_children(MarkStack&) {};
  virtual ~HeapEntity() = default;
};

class Symbol { 
private:
  friend struct std::hash<Symbol>;
public:
  const std::string *id = nullptr;
  const std::string& get_name() const {
    return *id; 
  }
  bool operator ==(const Symbol& other) const {
    return id == other.id; 
  }
};

class String : public HeapEntity {
public:
  std::string data;
  String(std::string data): data {std::move(data)} {}

  void push_children(MarkStack&) override;
};

class Cons : public HeapEntity {
public:
  Obj car;
  Obj cdr;
  Cons(Obj car, Obj cdr):
    car {std::move(car)},
    cdr {std::move(cdr)}
  {} 
  Obj at(const std::string&);
  void push_children(MarkStack&) override;
};

class Vector : public HeapEntity {
public:
  std::vector<Obj> data;
  Vector(std::vector<Obj> data): data {std::move(data)} {}
  void push_children(MarkStack&) override;
};

class Builtin : public HeapEntity {
private:
  std::function<Obj(const ArgList&, Interpreter&)> func;
public:
  Builtin(decltype(func) f): func {f} {};
  Obj operator()(const ArgList& args, Interpreter& interp) const {
    return func(args, interp);
  }
  void push_children(MarkStack&) override;
};

class Procedure : public HeapEntity {
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
  void push_children(MarkStack&) override;
};

template<class... Ts> 
struct Overloaded : Ts... { 
  using Ts::operator()...; 
};

inline bool is_bool(const Obj& obj) {return std::holds_alternative<bool>(obj);}
inline bool is_number(const Obj& obj) {return std::holds_alternative<double>(obj);}
inline bool is_char(const Obj& obj) {return std::holds_alternative<char>(obj);}
inline bool is_symbol(const Obj& obj){return std::holds_alternative<Symbol>(obj);}
inline bool is_string(const Obj& obj) {return std::holds_alternative<String*>(obj);}
inline bool is_pair(const Obj& obj) {return std::holds_alternative<Cons*>(obj);}
inline bool is_vector(const Obj& obj) {return std::holds_alternative<Vector*>(obj);}
inline bool is_primitive(const Obj& obj) {return std::holds_alternative<Builtin*>(obj);}
inline bool is_procedure(const Obj& obj) {return std::holds_alternative<Procedure*>(obj);}
inline bool is_callable(const Obj& obj) {return is_primitive(obj) || is_procedure(obj);}
inline bool is_null(const Obj& obj) {return std::holds_alternative<Null>(obj);}
inline bool is_void(const Obj& obj) {return std::holds_alternative<Void>(obj);}

inline bool same_type(const Obj obj_0, const Obj obj_1) {
  return obj_0.index() == obj_1.index();
}

inline bool& as_bool(Obj& obj) {return std::get<bool>(obj);}
inline const bool& as_bool(const Obj& obj) {return std::get<bool>(obj);}

inline double& as_number(Obj& obj) {return std::get<double>(obj);}
inline const double& as_number(const Obj& obj) {return std::get<double>(obj);}

inline char& as_char(Obj& obj) {return std::get<char>(obj);}
inline const char& as_char(const Obj& obj) {return std::get<char>(obj);}

inline Symbol& as_symbol(Obj& obj) {return std::get<Symbol>(obj);}
inline const Symbol& as_symbol(const Obj& obj) {return std::get<Symbol>(obj);}

inline String*& as_string(Obj& obj) {return std::get<String*>(obj);}
inline String* const& as_string(const Obj& obj) {return std::get<String*>(obj);}

inline Cons*& as_pair(Obj& obj) {return std::get<Cons*>(obj);}
inline Cons* const& as_pair(const Obj& obj) {return std::get<Cons*>(obj);}

inline Vector*& as_vector(Obj& obj) {return std::get<Vector*>(obj);}
inline Vector* const& as_vector(const Obj& obj) {return std::get<Vector*>(obj);}

inline Builtin*& as_primitive(Obj& obj) {return std::get<Builtin*>(obj);}
inline Builtin* const& as_primitive(const Obj& obj) {return std::get<Builtin*>(obj);}

inline Procedure*& as_procedure(Obj& obj) {return std::get<Procedure*>(obj);}
inline Procedure* const& as_procedure(const Obj& obj) {return std::get<Procedure*>(obj);}

inline bool is_true(const Obj& obj) {return (!is_bool(obj) || as_bool(obj) == true);}
inline bool is_false(const Obj& obj) {return !is_true(obj);}

inline bool operator ==(const Null&, const Null&) {return true;}
inline bool operator ==(const Void&, const Void&) {return true;}

std::pair<int, bool> list_profile(const Obj);

inline int list_length(const Obj cons) {return list_profile(cons).first;}
inline bool is_proper_list(const Obj cons) {return list_profile(cons).second;}
inline bool is_list(const Obj& obj) {
  return 
    is_null(obj) || 
    (is_pair(obj) && is_proper_list(as_pair(obj)));
}

bool equal(const Obj, const Obj);

std::string stringify(const Obj);
std::string stringify_type(const Obj);

}

namespace std {
  template<>
  struct hash<Scheme::Symbol> {
    size_t operator()(const Scheme::Symbol& s) const {
      return hash<const void*>()(s.id);
    }
  };
}
