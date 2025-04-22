#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "tco.hpp"
#include <unordered_map>

#include <iostream>

namespace Scheme {

using namespace std::string_literals;
static constexpr int MAXARGS = 256;

bool is_obj(const EvalResult& res) {return std::holds_alternative<Obj>(res); }
bool is_tailcall(const EvalResult& res) {return std::holds_alternative<TailCall>(res); }

Obj& as_obj(EvalResult& res) { return std::get<Obj>(res); }
const Obj& as_obj(const EvalResult& res) { return std::get<Obj>(res); }

TailCall& as_tailcall(EvalResult& res) { return std::get<TailCall>(res); }
const TailCall& as_tailcall(const EvalResult& res) { return std::get<TailCall>(res); }

int
Expression::get_size(Cons* obj) const {
  int sz = 0;
  while (obj != nullptr) {
    sz++;
    if (!is_pair(obj->cdr)) {
      break;
    }
    else {
      obj = as_pair(obj->cdr);
    }
  }
  return sz;
}

void
Expression::assert_size(Cons *obj, const int lb, const int ub) const {
  const int sz = get_size(obj);
  if (sz < lb || sz > ub) {
    throw std::runtime_error(
      name + 
      " expression is of wrong size [" + 
      std::to_string(sz) + 
      "]"
    );
  }
}

Expression::Expression(const std::string& n, Cons *obj, const int lb, const int ub):
  name {n} 
{
  if (lb != -1) {
    assert_size(obj, lb, ub);
  }
}

std::string
Expression::get_name() const {
  return name;
}

template<typename T, typename F>
static std::vector<T>
cons2vec(Obj ls, F fn) {
  std::vector<T> ret {};
  while (is_pair(ls)) {
    const auto as_cons = as_pair(ls);
    ret.push_back(fn(as_cons->car));
    ls = as_cons->cdr;
  }
  return ret;
}

static ParamList 
cons2symbols(Obj ls) {
  return cons2vec<Symbol>(ls, [](const Obj& o) -> const Symbol& {return as_symbol(o); });
}

static ExprList
cons2exprs(Obj ls) {
  return cons2vec<ExprPtr>(ls, [](const Obj& o) -> ExprPtr {
    return ExprPtr(classify(o));
  });
}

Literal::Literal(Obj obj):
  Expression("literal"s),
  obj {std::move(obj)}
{}

Variable::Variable(Symbol obj):
  Expression("variable"s),
  sym {std::move(obj)}
{}

Quoted::Quoted(Cons *obj):
  Expression("quoted"s, obj, 2, 2),
  text_of_quotation {obj->at("cadr")}
{}

Set::Set(Cons *obj):
  Expression("set!"s, obj, 3, 3)
{
  if (!is_symbol(obj->at("cadr"))) {
    throw std::runtime_error("tried to assign something to a non-variable");
  }
  variable = as_symbol(obj->at("cadr"));
  const auto cddr = obj->at("cddr");
  value = ExprPtr(classify(obj->at("caddr")));
}

If::If(Cons *obj):
  Expression("if"s, obj, 3, 4),
  predicate {classify(obj->at("cadr"))},
  consequent {classify(obj->at("caddr"))},
  alternative {
    !is_null(obj->at("cdddr"))
  ? ExprPtr(classify(obj->at("cadddr")))
  : ExprPtr(new Literal(false))
  }
{}

If::If(Expression *p, Expression *c, Expression *a):
  Expression("if"s),
  predicate {ExprPtr(p)},
  consequent {ExprPtr(c)},
  alternative {ExprPtr(a)} 
{}

If::If():
  Expression("if"s),
  predicate {ExprPtr(new Literal(Void {}))},
  consequent {ExprPtr(new Literal(Void {}))},
  alternative {ExprPtr(new Literal(Void {}))}
{}

Begin::Begin(ExprList seq):
  Expression("begin"s),
  actions {std::move(seq)}
{}

Lambda::Lambda(Cons *obj):
  Expression("lambda"s, obj, 3, MAXARGS),
  parameters {cons2symbols(obj->at("cadr"))},
  body {LambdaBody(combine_expr(obj->at("cddr")))}
{
  body->tco();
}

Lambda::Lambda(Obj parameters_, Obj body_):
  Expression("lambda"s),
  parameters {cons2symbols(parameters_)},
  body {LambdaBody(combine_expr(body_))}
{
  body->tco();
}

Define::Define(Cons *obj):
  Expression("define"s, obj, 3, MAXARGS)
{
  const auto cadr = obj->at("cadr");

  if (is_symbol(cadr)) {  
    variable = as_symbol(cadr);
    value = ExprPtr(classify(obj->at("caddr")));
  }

  else if (is_pair(cadr)) {
    const auto parameters = obj->at("cdadr");
    const auto body = obj->at("cddr");
    if (!is_symbol(obj->at("caadr"))) {
      throw std::runtime_error("procedure name must be a symbol");
    }
    variable = as_symbol(obj->at("caadr"));
    value = ExprPtr(new Lambda(parameters, body));
  }

  else {
    throw std::runtime_error("bad definition identifier");
  }
}

std::unordered_map<Symbol, ExprPtr>
Let::get_bindings(Obj li) {
  std::unordered_map<Symbol, ExprPtr> ret {};
  while (is_pair(li)) {
    const auto as_cons = as_pair(li);
    if (!is_pair(as_cons->car)) {
      throw std::runtime_error("Let::get_bindings: type error");
    }
    const auto car = as_pair(as_cons->car);
    if (!is_symbol(car->car)) {
      throw std::runtime_error("Let::get_bindings: type error");
    }
    
    ret.insert({
      as_symbol(car->at("car")),
      ExprPtr(classify(car->at("cadr")))
    });
    li = as_cons->cdr;
  }
  return ret;
}

Environment *
Let::get_frame(Environment *env) const {
  auto ret = new Environment (env);
  for (const auto& p : bindings) {
    ret->define_variable(
      p.first,
      as_obj(p.second->eval(env))
    );
  }
  return ret;
}

Let::Let(Cons *obj):
  Expression("let"s, obj, 3, MAXARGS),
  bindings {get_bindings(obj->at("cadr"))},
  body {ExprPtr(combine_expr(obj->at("cddr")))}
{}


bool
Clause::is_else_clause(Obj obj) const {
  return
    is_symbol(obj) &&
    as_symbol(obj).get_name() == "else";
}

Clause::Clause(Cons *obj) {
  if (is_else_clause(obj->car)) {
    is_else = 1;
    predicate = ExprPtr(new Literal(true));
  } 
  else {
    is_else = 0;
    predicate = ExprPtr(classify(obj->car));
  }
  actions = ExprPtr(combine_expr(obj->cdr));
}

If*
Cond::cond2if() const {
  If* ret = new If;
  /*
  for (auto curr = clauses.rbegin(); curr != clauses.rend(); curr++) {
    ret = new If(curr->predicate, curr->actions, ret);
  }
  */
  return ret;
}

Cond::Cond(Obj obj):
  Expression("cond"s) 
{
  obj = as_pair(obj)->cdr;
  /*
  while (is_pair(obj)) {
    const auto as_cons = as_pair(obj);
    if (!is_pair(as_cons->car)) {
      throw std::runtime_error("cond type error\n");
    }
    const auto new_clause = Clause(as_pair(as_cons->car));
    clauses.push_back(new_clause);
    if (new_clause.is_else)
      break;
    obj = as_cons->cdr;
  }
  */
  if_form = ExprPtr(cond2if());
}

Application::Application(Cons *obj):
  Expression("application"s),
  op {ExprPtr(classify(obj->car))},
  params {cons2exprs(obj->cdr)}
{}

And::And(Cons *obj):
  Expression("and"s, obj, 2, MAXARGS),
  exprs {cons2exprs(obj->cdr)}
{}

Or::Or(Cons *obj):
  Expression("or"s, obj, 2, MAXARGS),
  exprs {cons2exprs(obj->cdr)}
{}

Cxr::Cxr(Symbol tag, Cons *obj): 
  Expression(tag.get_name(), obj, 2, 2),
  word {tag}, 
  expr {ExprPtr(classify(obj->at("cadr")))} 
{}

Expression*
combine_expr(Obj seq) {
  if (is_null(seq)) {
    return new Literal(Void {});
  }
  else if (is_null(as_pair(seq)->cdr)) {
    return classify(as_pair(seq)->car);
  }
  else {
    return new Begin(cons2exprs(seq));
  }
}

static Expression* make_quoted(Cons *obj) { return new Quoted(obj); }
static Expression* make_set(Cons *obj) { return new Set(obj); }
static Expression* make_define(Cons *obj) { return new Define(obj); }
static Expression* make_if(Cons *obj) { return new If(obj); }
static Expression* make_lambda(Cons *obj) { return new Lambda(obj); }
static Expression* make_let(Cons *obj) { return new Let(obj); }
static Expression* make_begin(Cons *obj) { return combine_expr(obj->cdr); }
static Expression* make_cond(Cons *obj) { return new Cond(obj); }
static Expression* make_application(Cons *obj) { return new Application(obj); }
static Expression* make_and(Cons *obj) { return new And(obj); }
static Expression* make_or(Cons *obj) { return new Or(obj); }

static std::unordered_map<Symbol, Expression*(*)(Cons*)> 
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
  for (int i = 1; i < s.size() - 1; i++) {
    if (s[i] != 'a' && s[i] != 'd') {
      return false;
    }
  }
  return true;
}

Expression*
classify(const Obj& obj) {
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
        return new Cxr(tag, p);
      }
    }
    return new Application(p);
  }
  else if (is_symbol(obj))
    return new Variable(as_symbol(obj));
  else
    return new Literal(obj);
}

}
