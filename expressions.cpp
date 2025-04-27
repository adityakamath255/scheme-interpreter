#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include <unordered_map>
#include <iostream>
#include <format>

namespace Scheme {

void 
If::tco() {
  consequent->tco(); 
  alternative->tco();
}

void 
Begin::tco() {
  if (!actions.empty()) {
    actions.back()->tco();
  }
}

void 
Cond::tco() {
  for (auto& clause : clauses) {
    clause.actions->tco();
  }
}

void 
Application::tco() {
  at_tail = true;
}

static constexpr int MAXARGS = 256;

static int
get_size(Cons* cons) {
  int sz = 0;
  while (cons != nullptr) {
    sz++;
    if (!is_pair(cons->cdr)) {
      break;
    }
    else {
      cons = as_pair(cons->cdr);
    }
  }
  return sz;
}

static void
assert_size(Cons *cons, const int lb, const int ub, const std::string& name) {
  const int sz = get_size(cons);
  if (sz < lb || sz > ub) {
    throw std::runtime_error(std::format("{} expression is of wrong size [{}]", name, sz));
  }
}

template<typename T, typename F>
static std::vector<T>
cons2vec(const Obj& ls, F fn) {
  std::vector<T> ret {};
  Obj current = ls;
  while (is_pair(current)) {
    const auto as_cons = as_pair(current);
    ret.push_back(fn(as_cons->car));
    current = as_cons->cdr;
  }
  return ret;
}

static ParamList
cons2symbols(const Obj& ls) {
  std::vector<Symbol> ret {};
  Obj curr = ls;
  while (is_pair(curr)) {
    const auto as_cons = as_pair(curr);
    ret.push_back(as_symbol(as_cons->car));
    curr = as_cons->cdr;
  }
  return ret;
}

static ExprList
cons2exprs(const Obj& ls, Interpreter& interp) {
  std::vector<Expression*> ret {};
  Obj curr = ls;
  while (is_pair(curr)) {
    const auto as_cons = as_pair(curr);
    ret.push_back(build_ast(as_cons->car, interp));
    curr = as_cons->cdr;
  }
  return ret;
}

static Expression*
make_quoted(Cons *cons, Interpreter& interp) {
  assert_size(cons, 2, 2, "quoted");
  return interp.alloc.make<Quoted>(cons->at("cadr"));
}

static Expression*
make_set(Cons *cons, Interpreter& interp) {
  assert_size(cons, 3, 3, "set!");
  if (!is_symbol(cons->at("cadr"))) {
    throw std::runtime_error("tried to assign something to a non-variable");
  }
  const auto variable = as_symbol(cons->at("cadr"));
  const auto cddr = cons->at("cddr");
  auto value = build_ast(cons->at("caddr"), interp);
  return interp.alloc.make<Set>(variable, value);
}

static Expression*
make_if(Cons *cons, Interpreter& interp) {
  assert_size(cons, 3, 4, "if");
  return interp.alloc.make<If>(
    build_ast(cons->at("cadr"), interp),
    build_ast(cons->at("caddr"), interp),
      !is_null(cons->at("cdddr"))
    ? build_ast(cons->at("cadddr"), interp)
    : interp.alloc.make<Literal>(false)
  );
}

static Expression*
make_lambda(Cons *cons, Interpreter& interp) {
  assert_size(cons, 3, MAXARGS, "lambda");
  auto ret = interp.alloc.make<Lambda>(
    std::move(cons2symbols(cons->at("cadr"))),
    std::move(combine_expr(cons->at("cddr"), interp))
  );
  ret->body->tco();
  return ret;
}

static Expression*
make_lambda(Obj parameters, Obj body, Interpreter& interp) {
  auto ret = interp.alloc.make<Lambda>(
    std::move(cons2symbols(parameters)),
    std::move(combine_expr(body, interp))
  );
  ret->body->tco();
  return ret;
}

static Expression*
make_define(Cons *cons, Interpreter& interp) {
  assert_size(cons, 3, MAXARGS, "define");
  const auto cadr = cons->at("cadr");

  if (is_symbol(cadr)) {
    return interp.alloc.make<Define>(
      as_symbol(cadr), 
      build_ast(cons->at("caddr"), interp)
    );
  }

  else if (is_pair(cadr)) {
    const auto parameters = cons->at("cdadr");
    const auto body = cons->at("cddr");
    if (!is_symbol(cons->at("caadr"))) {
      throw std::runtime_error("procedure name must be a symbol");
    }
    return interp.alloc.make<Define>(
      as_symbol(cons->at("caadr")),
      make_lambda(parameters, body, interp)
    );
  }

  else {
    throw std::runtime_error("bad definition identifier");
  }
}

static std::unordered_map<Symbol, Expression*>
get_bindings(Obj li, Interpreter& interp) {
  std::unordered_map<Symbol, Expression*> ret {};
  while (is_pair(li)) {
    const auto as_cons = as_pair(li);
    if (!is_pair(as_cons->car)) {
      throw std::runtime_error("let bindings must be represented as pairs");
    }
    const auto car = as_pair(as_cons->car);
    if (!is_symbol(car->car)) {
      throw std::runtime_error("let bindings must be to variables");
    }
    
    ret.emplace(
      as_symbol(car->at("car")),
      build_ast(car->at("cadr"), interp)
    );

    li = as_cons->cdr;
  }
  return ret;
}

static Expression*
make_let(Cons *cons, Interpreter& interp) {
  assert_size(cons, 3, MAXARGS, "let");
  return interp.alloc.make<Let>(
    std::move(get_bindings(cons->at("cadr"), interp)),
    combine_expr(cons->at("cddr"), interp)
  );
}

static Expression*
make_begin(Cons *cons, Interpreter& interp) {
  return interp.alloc.make<Begin>(cons2exprs(cons, interp));
}

static bool
is_else_clause(const Obj& obj) {
  return
    is_symbol(obj) &&
    as_symbol(obj).get_name() == "else";
}

static Clause
make_clause(Cons *cons, Interpreter& interp) {
  Clause ret {};
  if (is_else_clause(cons->car)) {
    ret.is_else = true;
    ret.predicate = interp.alloc.make<Literal>(true);
  } 
  else {
    ret.is_else = false;
    ret.predicate = build_ast(cons->car, interp);
  }
  ret.actions = combine_expr(cons->cdr, interp);
  return ret;
}

static Expression*
make_cond(Cons *cons, Interpreter& interp) {
  std::vector<Clause> clauses {};
  Obj obj = cons->cdr;
  while (is_pair(obj)) {
    const auto as_cons = as_pair(obj);
    if (!is_pair(as_cons->car)) {
      throw std::runtime_error("bad form for cond expression\n");
    }
    clauses.push_back(make_clause(as_pair(as_cons->car), interp));
    if (clauses.back().is_else) {
      if (!is_null(as_cons->cdr)) {
        throw std::runtime_error("no clauses allowed after else clause\n");
      }
      else {
        break;
      }
    }
    obj = as_cons->cdr;
  }
  return interp.alloc.make<Cond>(std::move(clauses));
}

static Expression*
make_application(Cons *cons, Interpreter& interp) {
  return interp.alloc.make<Application>(
    build_ast(cons->car, interp),
    std::move(cons2exprs(cons->cdr, interp))
  );
}

static Expression*
make_and(Cons *cons, Interpreter& interp) {
  return interp.alloc.make<And>(std::move(cons2exprs(cons->cdr, interp)));
}

static Expression*
make_or(Cons *cons, Interpreter& interp) {
  return interp.alloc.make<Or>(std::move(cons2exprs(cons->cdr, interp)));
}

static Expression*
make_cxr(Symbol tag, Cons *cons, Interpreter& interp) {
  assert_size(cons, 2, 2, tag.get_name());
  return interp.alloc.make<Cxr>(tag, build_ast(cons->at("cadr"), interp));
}

Expression*
combine_expr(const Obj& seq, Interpreter& interp) {
  if (is_null(seq)) {
    return interp.alloc.make<Literal>(Void {});
  }
  else if (is_null(as_pair(seq)->cdr)) {
    return build_ast(as_pair(seq)->car, interp);
  }
  else {
    return interp.alloc.make<Begin>(cons2exprs(seq, interp));
  }
}

static std::unordered_map<std::string, Expression*(*)(Cons*, Interpreter&)> 
special_forms = {
  {"quote", make_quoted},
  {"set!", make_set},
  {"define", make_define},
  {"if", make_if},
  {"lambda", make_lambda},
  {"let", make_let},
  {"begin", make_begin},
  {"cond", make_cond},
  {"and", make_and},
  {"or", make_or},
};

static bool
is_cxr(const std::string& s) {
  if (s.front() != 'c' || s.back() != 'r') {
    return false;
  }
  for (size_t i = 1; i < s.size() - 1; i++) {
    if (s[i] != 'a' && s[i] != 'd') {
      return false;
    }
  }
  return true;
}

Expression*
build_ast(const Obj& obj, Interpreter& interp) {
  if (is_pair(obj)) {
    const auto p = as_pair(obj);
    if (is_symbol(p->car)) {
      const auto tag = as_symbol(p->car);
      const auto found = special_forms.find(tag.get_name());
      if (found != special_forms.end()) {
        const auto func = found->second;
        return func(p, interp);
      }
      else if (is_cxr(tag.get_name())) {
        return make_cxr(tag, p, interp);
      }
    }
    return make_application(p, interp);
  }
  else if (is_symbol(obj))
    return interp.alloc.make<Variable>(as_symbol(obj));
  else
    return interp.alloc.make<Literal>(obj);
}

}
