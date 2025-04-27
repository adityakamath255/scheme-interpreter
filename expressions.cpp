#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include <unordered_map>
#include <iostream>
#include <format>

namespace Scheme {

using namespace std::string_literals;
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
  return cons2vec<Symbol>(ls, [](const Obj& obj) -> Symbol { return as_symbol(obj); });
}

static ExprList
cons2exprs(const Obj& ls) {
  return cons2vec<Expression*>(ls, build_ast);
}

static Expression*
make_quoted(Cons *cons) {
  assert_size(cons, 2, 2, "quoted");
  return new Quoted(cons->at("cadr"));
}

static Expression*
make_set(Cons *cons) {
  assert_size(cons, 3, 3, "set!");
  if (!is_symbol(cons->at("cadr"))) {
    throw std::runtime_error("tried to assign something to a non-variable");
  }
  const auto variable = as_symbol(cons->at("cadr"));
  const auto cddr = cons->at("cddr");
  auto value = build_ast(cons->at("caddr"));
  return new Set(variable, value);
}

static Expression*
make_if(Cons *cons) {
  assert_size(cons, 3, 4, "if");
  return new If(
    build_ast(cons->at("cadr")),
    build_ast(cons->at("caddr")),
      !is_null(cons->at("cdddr"))
    ? build_ast(cons->at("cadddr"))
    : new Literal (false)
  );
}

static Expression*
make_lambda(Cons *cons) {
  assert_size(cons, 3, MAXARGS, "lambda");
  auto ret = new Lambda(
    std::move(cons2symbols(cons->at("cadr"))),
    std::move(combine_expr(cons->at("cddr")))
  );
  ret->body->tco();
  return ret;
}

static Expression*
make_lambda(Obj parameters, Obj body) {
  auto ret = new Lambda(
    std::move(cons2symbols(parameters)),
    std::move(combine_expr(body))
  );
  ret->body->tco();
  return ret;
}

static Expression*
make_define(Cons *cons) {
  assert_size(cons, 3, MAXARGS, "define");
  const auto cadr = cons->at("cadr");

  if (is_symbol(cadr)) {
    return new Define(
      as_symbol(cadr), 
      build_ast(cons->at("caddr"))
    );
  }

  else if (is_pair(cadr)) {
    const auto parameters = cons->at("cdadr");
    const auto body = cons->at("cddr");
    if (!is_symbol(cons->at("caadr"))) {
      throw std::runtime_error("procedure name must be a symbol");
    }
    return new Define(
      as_symbol(cons->at("caadr")),
      make_lambda(parameters, body)
    );
  }

  else {
    throw std::runtime_error("bad definition identifier");
  }
}

static std::unordered_map<Symbol, Expression*>
get_bindings(Obj li) {
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
    
    ret.insert({
      as_symbol(car->at("car")),
      build_ast(car->at("cadr"))
    });

    li = as_cons->cdr;
  }
  return ret;
}

static Expression*
make_let(Cons *cons) {
  assert_size(cons, 3, MAXARGS, "let");
  return new Let(
    std::move(get_bindings(cons->at("cadr"))),
    combine_expr(cons->at("cddr"))
  );
}

static Expression*
make_begin(Cons *cons) {
  return new Begin(cons2exprs(cons));
}

static bool
is_else_clause(const Obj& obj) {
  return
    is_symbol(obj) &&
    as_symbol(obj).get_name() == "else";
}

static Clause
make_clause(Cons *cons) {
  Clause ret {};
  if (is_else_clause(cons->car)) {
    ret.is_else = true;
    ret.predicate = new Literal(true);
  } 
  else {
    ret.is_else = false;
    ret.predicate = build_ast(cons->car);
  }
  ret.actions = combine_expr(cons->cdr);
  return ret;
}

static Expression*
make_cond(Cons *cons) {
  std::vector<Clause> clauses {};
  Obj obj = cons->cdr;
  while (is_pair(obj)) {
    const auto as_cons = as_pair(obj);
    if (!is_pair(as_cons->car)) {
      throw std::runtime_error("bad form for cond expression\n");
    }
    clauses.push_back(make_clause(as_pair(as_cons->car)));
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
  return new Cond(std::move(clauses));
}

static Expression*
make_application(Cons *cons) {
  return new Application(
    build_ast(cons->car),
    std::move(cons2exprs(cons->cdr))
  );
}

static Expression*
make_and(Cons *cons) {
  return new And(std::move(cons2exprs(cons->cdr)));
}

static Expression*
make_or(Cons *cons) {
  return new Or(std::move(cons2exprs(cons->cdr)));
}

static Expression*
make_cxr(Symbol tag, Cons *cons) {
  assert_size(cons, 2, 2, tag.get_name());
  return new Cxr(tag, build_ast(cons->at("cadr")));
}

Expression*
combine_expr(const Obj& seq) {
  if (is_null(seq)) {
    return new Literal(Void {});
  }
  else if (is_null(as_pair(seq)->cdr)) {
    return build_ast(as_pair(seq)->car);
  }
  else {
    return new Begin(cons2exprs(seq));
  }
}

static std::unordered_map<std::string, Expression*(*)(Cons*)> 
special_forms = {
  {"quote"s, make_quoted},
  {"set!"s, make_set},
  {"define"s, make_define},
  {"if"s, make_if},
  {"lambda"s, make_lambda},
  {"let"s, make_let},
  {"begin"s, make_begin},
  {"cond"s, make_cond},
  {"and"s, make_and},
  {"or"s, make_or},
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
build_ast(const Obj& obj) {
  if (is_pair(obj)) {
    const auto p = as_pair(obj);
    if (is_symbol(p->car)) {
      const auto tag = as_symbol(p->car);
      const auto found = special_forms.find(tag.get_name());
      if (found != special_forms.end()) {
        const auto func = found->second;
        return func(p);
      }
      else if (is_cxr(tag.get_name())) {
        return make_cxr(tag, p);
      }
    }
    return make_application(p);
  }
  else if (is_symbol(obj))
    return new Variable(as_symbol(obj));
  else
    return new Literal(obj);
}

}
