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

using Obj = std::variant<
  bool,
  double,
  Symbol,
  string,
  Cons*,
  Primitive*,
  Procedure*,
  nullptr_t
>;

struct Symbol {
  string name;
  Symbol();
  Symbol(const string& s);
};

bool operator ==(const Symbol&, const Symbol&);
bool operator <(const Symbol&, const Symbol&);

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

class Environment {
private:
  std::map<Symbol, Obj> frame {};
  Environment *const super;
  decltype(frame)::iterator assoc(const Symbol& s);
public:
  Environment();
  Environment(Environment *super_);
  void set_variable(const Symbol& s, const Obj obj);
  Obj lookup(const Symbol& s);
  void define_variable(const Symbol& s, Obj obj);
  Environment *extend(const vector<Symbol>& parameters, const vector<Obj>& arguments);
};

struct TailCall {
  Obj proc;
  vector<Obj> args;
  TailCall(Obj& proc, vector<Obj>& args);
};  

class Expression {
private:
  int get_size(Cons* obj) const;
  void assert_size(Cons *obj, const int lb, const int ub) const;
protected:
  string name;
public:
  Expression(const string& n, Cons *obj = nullptr, const int lb = -1, const int ub = -1);
  string get_name() const;
  virtual Obj eval(Environment*) const = 0;
  virtual void tco() {}
};

bool is_pair(const Obj obj);
bool is_null(const Obj obj);
bool is_true(const Obj obj);
bool is_false(const Obj obj);
Obj eval(Expression *expr, Environment *const env);
Obj apply(Obj p, vector<Obj> args);

constexpr double precision = 1e-9;

}
