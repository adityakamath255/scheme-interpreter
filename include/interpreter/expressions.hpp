#pragma once
#include <interpreter/types.hpp>
#include <interpreter/environment.hpp>
#include <interpreter/interpreter.hpp>
#include <variant>

namespace Scheme {

class Interpreter;

struct TailCall {
  Obj proc;
  ArgList args;
  TailCall(Obj p, ArgList ls):
    proc {std::move(p)},
    args {std::move(ls)}
  {}
};

using EvalResult = std::variant<
  Obj,
  TailCall
>;

using ExprList = std::vector<Expression*>;

inline bool is_obj(const EvalResult& res) {return std::holds_alternative<Obj>(res); }
inline bool is_tailcall(const EvalResult& res) {return std::holds_alternative<TailCall>(res); }

inline Obj& as_obj(EvalResult& res) { return std::get<Obj>(res); }
inline const Obj& as_obj(const EvalResult& res) { return std::get<Obj>(res); }

inline TailCall& as_tailcall(EvalResult& res) { return std::get<TailCall>(res); }
inline const TailCall& as_tailcall(const EvalResult& res) { return std::get<TailCall>(res); }

class Expression : public HeapEntity {
public:
  Expression() = default;
  virtual ~Expression() = default;
  virtual EvalResult eval(Environment*, Interpreter&) = 0;
  virtual void tco() {}
};

struct Literal : public Expression {
  Obj obj;
  explicit Literal(Obj o): obj(std::move(o)) {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Variable : public Expression {
  Symbol sym;
  int depth;
  bool resolved;
  explicit Variable(Symbol s, int d = 0, bool r = false): sym(s), depth(d), resolved(r) {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Quoted : public Expression {
  Obj text;
  explicit Quoted(Obj text): text {text} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Quasiquoted : public Expression {
  std::variant<std::vector<Expression*>, Obj> text;
  explicit Quasiquoted(std::vector<Expression*> exprs): text {std::move(exprs)} {}
  explicit Quasiquoted(Obj obj): text {obj} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Set : public Expression {
  Symbol variable;
  Expression *value;
  Set(Symbol var, Expression *val): variable {std::move(var)}, value {val} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct If : public Expression {
  Expression *predicate;
  Expression *consequent;
  Expression *alternative;
  If(Expression *p, Expression *c, Expression *a): predicate {p}, consequent {c}, alternative {a} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override;
  void push_children(MarkStack&) override;
};

struct Begin : public Expression {
  ExprList actions;
  Begin(ExprList a): actions {std::move(a)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override;
  void push_children(MarkStack&) override;
};

struct Lambda : public Expression {
  ParamList parameters;
  Expression *body;
  bool is_variadic;
  Lambda(ParamList p, Expression *b, bool v): 
    parameters {std::move(p)}, 
    body {b},
    is_variadic {v}
  {
    body->tco();
  }
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Define : public Expression {
  Symbol variable;
  Expression *value;
  Define(Symbol var, Expression *val): variable {std::move(var)}, value {val} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

using LetBindings = std::vector<std::pair<Symbol, Expression*>>;

struct Let : public Expression {
  LetBindings bindings;
  Expression *body;
  Let(decltype(bindings) bn, Expression *bd): bindings {std::move(bn)}, body {bd} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct LetSeq : public Expression {
  LetBindings bindings;
  Expression *body;
  LetSeq(decltype(bindings) bn, Expression *bd): bindings {std::move(bn)}, body {bd} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Clause {
  bool is_else;
  Expression *predicate;
  Expression *actions;
};

struct Cond : public Expression {
  std::vector<Clause> clauses;
  Cond(decltype(clauses) c): clauses {std::move(c)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override;
  void push_children(MarkStack&) override;
};

struct Application : public Expression {
  Expression *op;
  ExprList params;
  bool at_tail = false;
  Application(Expression *o, ExprList p): op {o}, params {std::move(p)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void tco() override;
  void push_children(MarkStack&) override;
};

struct And : public Expression {
  ExprList exprs;
  And(ExprList e): exprs {std::move(e)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

struct Or : public Expression {
  ExprList exprs;
  Or(ExprList e): exprs {std::move(e)} {}
  EvalResult eval(Environment*, Interpreter&) override;
  void push_children(MarkStack&) override;
};

Expression *combine_expr(const Obj&, Interpreter&);
Expression *build_ast(const Obj&, Interpreter&);

}
