#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include <unordered_map>
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

void
assert_size(Cons *cons, const int lb, const int ub, const std::string& name) {
  const auto [length, proper] = list_profile(cons);
  if (!proper) {
    throw std::runtime_error(std::format("{} expression is an improper list", name));
  }
  if (length < lb || length > ub) {
    throw std::runtime_error(std::format("{} expression is of wrong size [{}]", name, length));
  }
}

static std::pair<ParamList, bool>
cons2paramlist(const Obj& ls) {
  std::vector<Symbol> ret {};
  Obj curr = ls;
  while (is_pair(curr)) {
    const auto cons = as_pair(curr);
    const auto car = cons->car;
    if (is_symbol(car)) {
      ret.push_back(as_symbol(car));
    }
    else {
      throw std::runtime_error(std::format("all parameters must be symbols: {}", stringify(ls)));
    }
    curr = cons->cdr;
  }
  if (is_null(curr)) {
    return {ret, false};
  }
  else if (is_symbol(curr)) {
    ret.push_back(as_symbol(curr));
    return {ret, true};
  }
  else {
    throw std::runtime_error(std::format("all parameters must be symbols: {}", stringify(ls)));
  }
}

static ExprList
cons2exprs(const Obj& ls, Interpreter& interp) {
  std::vector<Expression*> ret {};
  Obj curr = ls;
  while (is_pair(curr)) {
    const auto cons = as_pair(curr);
    ret.push_back(build_ast(cons->car, interp));
    curr = cons->cdr;
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
  const auto cdr = as_pair(cons->cdr);
  const auto cddr = as_pair(cdr->cdr);
  if (!is_symbol(cdr->car)) {
    throw std::runtime_error("tried to assign something to a non-variable");
  }
  const auto variable = as_symbol(cdr->car);
  auto value = build_ast(cddr->car, interp);
  return interp.alloc.make<Set>(variable, value);
}

static Expression*
make_if(Cons *cons, Interpreter& interp) {
  assert_size(cons, 3, 4, "if");
  const auto cdr = as_pair(cons->cdr);
  const auto cddr = as_pair(cdr->cdr);
  return interp.alloc.make<If>(
    build_ast(cdr->car, interp),
    build_ast(cddr->car, interp),
      is_pair(cddr->cdr)
    ? build_ast(as_pair(cddr->cdr)->car, interp)
    : interp.alloc.make<Literal>(Void {})
  );
}

static Expression*
make_lambda(const Obj& params_cons, const Obj& body_cons, Interpreter& interp) {
  auto [params, is_variadic] = cons2paramlist(params_cons);
  auto body = combine_expr(body_cons, interp);
  auto ret = interp.alloc.make<Lambda>(
    std::move(params),
    body,
    is_variadic
  );
  return ret;
}

static Expression*
make_lambda(Cons *cons, Interpreter& interp) {
  assert_size(cons, 2, MAXARGS, "lambda");
  const auto cdr = as_pair(cons->cdr);
  return make_lambda(cdr->car, cdr->cdr, interp);
}

static Expression*
make_var_define(Cons *cons, Cons *cdr, Interpreter& interp) {
  const auto name = as_symbol(cdr->car);
  if (is_null(cdr->cdr)) {
    return interp.alloc.make<Define>(name, interp.alloc.make<Literal>(Void {}));
  }
  else {
    const auto cddr = as_pair(cdr->cdr);
    if (is_pair(cddr->cdr)) {
      throw std::runtime_error(std::format(
        "define expression {} is of wrong size [{}]",
        stringify(cons),
        list_length(cons)
      ));
    }
    return interp.alloc.make<Define>(name, build_ast(cddr->car, interp));
  }
}

static Expression*
make_proc_define(Cons *cons, Cons *cdr, Interpreter& interp) {
  const auto cadr = as_pair(cdr->car);
  const auto name = as_symbol(cadr->car);
  const auto parameters = cadr->cdr;
  const auto body = cdr->cdr; 
  if (!is_symbol(name)) {
    throw std::runtime_error(std::format(
      "in define expression {}, procedure name must be a symbol",
      stringify(cons)
    ));
  }
  return interp.alloc.make<Define>(name, make_lambda(parameters, body, interp));
}

static Expression*
make_define(Cons *cons, Interpreter& interp) {
  assert_size(cons, 2, MAXARGS, "define");
  const auto cdr = as_pair(cons->cdr);
  if (is_symbol(cdr->car)) {
    return make_var_define(cons, cdr, interp);
  }
  else if (is_pair(cdr->car)) {
    return make_proc_define(cons, cdr, interp);
  }
  else {
    throw std::runtime_error(std::format(
      "bad definition identifier: {}",
      stringify(cdr->car)
    ));
  }
}

static LetBindings
get_bindings(const Obj& obj, Interpreter& interp) {
  LetBindings ret {};
  auto ls = obj;

  while (is_pair(ls)) {
    const auto cons = as_pair(ls);

    if (!is_pair(cons->car)) {
      throw std::runtime_error("let bindings must be represented as 2-element lists");
    }

    const auto car = as_pair(cons->car);

    if (!is_pair(car->cdr)) {
      throw std::runtime_error("let bindings must be represented as 2-element lists");
    }

    const auto cdar = as_pair(car->cdr);

    if (!is_symbol(car->car)) {
      throw std::runtime_error("let bindings must be to variables");
    }
    
    const auto name = as_symbol(car->car);

    if (!is_null(cdar->cdr)) {
      throw std::runtime_error("let bindings must be represented as 2-element lists");
    }
 
    const auto expr = cdar->car;

    ret.emplace_back(name, build_ast(expr, interp));
    ls = cons->cdr;
  }
  if (!is_null(ls)) {
    throw std::runtime_error("let bindings must be a proper list");
  }
  return ret;
}

static Expression*
make_let(Cons *cons, Interpreter& interp) {
  assert_size(cons, 2, MAXARGS, "let");
  const auto cdr = as_pair(cons->cdr);
  return interp.alloc.make<Let>(
    get_bindings(cdr->car, interp),
    combine_expr(cdr->cdr, interp)
  );
}

static Expression*
make_let_seq(Cons *cons, Interpreter& interp) {
  assert_size(cons, 2, MAXARGS, "let*");
  const auto cdr = as_pair(cons->cdr);
  return interp.alloc.make<LetSeq>(
    get_bindings(cdr->car, interp),
    combine_expr(cdr->cdr, interp)
  );
}

static Expression*
make_begin(Cons *cons, Interpreter& interp) {
  assert_size(cons, 1, MAXARGS, "begin");
  return interp.alloc.make<Begin>(cons2exprs(cons->cdr, interp));
}

static Clause
make_clause(Cons *cons, Interpreter& interp) {
  Clause ret {};

  ret.is_else = is_symbol(cons->car) && as_symbol(cons->car).get_name() == "else";

  if (!ret.is_else) {
    ret.predicate = build_ast(cons->car, interp);
  }
  
  ret.has_actions = !is_null(cons->cdr);

  if (ret.is_else && !ret.has_actions) {
    throw std::runtime_error("else clause must have actions");
  }

  if (ret.has_actions) {
    ret.actions = combine_expr(cons->cdr, interp);
  }

  return ret;
}

static Expression*
make_cond(Cons *cons, Interpreter& interp) {
  assert_size(cons, 1, MAXARGS, "cond");
  std::vector<Clause> clauses {};
  Obj obj = cons->cdr;
  while (is_pair(obj)) {
    const auto cons = as_pair(obj);
    if (!is_pair(cons->car)) {
      throw std::runtime_error("bad form for cond expression");
    }
    clauses.push_back(make_clause(as_pair(cons->car), interp));
    if (clauses.back().is_else) {
      if (!is_null(cons->cdr)) {
        throw std::runtime_error("no clauses allowed after else clause");
      }
      else {
        return interp.alloc.make<Cond>(std::move(clauses));
      }
    }
    obj = cons->cdr;
  }
  if (!is_null(obj)) {
    throw std::runtime_error("cond expression is an improper list");
  }
  return interp.alloc.make<Cond>(std::move(clauses));
}

static Expression*
make_application(Cons *cons, Interpreter& interp) {
  assert_size(cons, 1, MAXARGS, std::format("{} application", stringify(cons->car)));
  return interp.alloc.make<Application>(
    build_ast(cons->car, interp),
    cons2exprs(cons->cdr, interp)
  );
}

static Expression*
make_and(Cons *cons, Interpreter& interp) {
  assert_size(cons, 0, MAXARGS, "and");
  return interp.alloc.make<And>(cons2exprs(cons->cdr, interp));
}

static Expression*
make_or(Cons *cons, Interpreter& interp) {
  assert_size(cons, 0, MAXARGS, "or");
  return interp.alloc.make<Or>(cons2exprs(cons->cdr, interp));
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
  {"let*", make_let_seq},
  {"letrec", make_let}, // they're the same in this implementation
  {"letrec*", make_let_seq}, 
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
