#include "expressions.hpp"

namespace Scheme {

using namespace std::string_literals;
static constexpr int MAXARGS = 256;

template<typename T, T (*f)(Obj)>
static vector<T>
cons2vec(Obj ls) {
  vector<T> ret {};
  while (is_pair(ls)) {
    const auto as_cons = get<Cons*>(ls);
    ret.push_back(f(as_cons->car));
    ls = as_cons->cdr;
  }
  return ret;
}

static Symbol as_symbol(Obj obj) {return get<Symbol>(obj); }

static vector<Symbol> 
cons2symbols(Obj ls) {
  return cons2vec<Symbol, as_symbol>(ls);
}

static vector<Expression*>
cons2exprs(Obj ls) {
  return cons2vec<Expression*, classify>(ls);
}

Literal::Literal(Obj obj):
  Expression("self-evaluating"s),
  obj {obj}
{}

Variable::Variable(Symbol& obj):
  Expression("Variable"s),
  sym {obj}
{}

Quoted::Quoted(Cons *obj):
  Expression("quoted"s, obj, 2, 2),
  text_of_quotation {obj->at("cadr")}
{}

Set::Set(Cons *obj):
  Expression("Set"s, obj, 3, 3)
{
  if (!holds_alternative<Symbol>(obj->at("cadr"))) {
    throw runtime_error("tried to assign something to a non-variable");
  }
  variable = get<Symbol>(obj->at("cadr"));
  const auto cddr = obj->at("cddr");
  value = classify(obj->at("caddr"));
}

If::If(Cons *obj):
  Expression("if"s, obj, 3, 4),
  predicate {classify(obj->at("cadr"))},
  consequent {classify(obj->at("caddr"))},
  alternative {
    !is_null(obj->at("cdddr"))
  ? classify(obj->at("cadddr"))
  : new Literal(false)
  }
{}

If::If(Expression *p, Expression *c, Expression *a):
  Expression("if"s),
  predicate {p},
  consequent {c},
  alternative {a} 
{}

If::If():
  Expression("if"s),
  predicate {new Literal(false)},
  consequent {new Literal(false)},
  alternative {new Literal(false)}
{}

Begin::Begin(const vector<Expression*>& seq):
  Expression("begin"s),
  actions {seq}
{}

Lambda::Lambda(Cons *obj):
  Expression("lambda"s, obj, 3, MAXARGS),
  parameters {cons2symbols(obj->at("cadr"))},
  body {combine_expr(obj->at("cddr"))}
{
  body->tco();
}

Lambda::Lambda(Obj parameters_, Obj body_):
  Expression("lambda"s),
  parameters {cons2symbols(parameters_)},
  body {combine_expr(body_)}
{
  body->tco();
}

Define::Define(Cons *obj):
  Expression("define"s, obj, 3, MAXARGS)
{
  const auto cadr = obj->at("cadr");

  if (holds_alternative<Symbol>(cadr)) {  
    variable = get<Symbol>(cadr);
    value = classify(obj->at("caddr"));
  }

  else if (holds_alternative<Cons*>(cadr)) {
    const auto parameters = obj->at("cdadr");
    const auto body = obj->at("cddr");
    if (!holds_alternative<Symbol>(obj->at("caadr"))) {
      throw runtime_error("procedure name must be a symbol");
    }
    variable = get<Symbol>(obj->at("caadr"));
    value = new Lambda(parameters, body);
  }

  else {
    throw runtime_error("bad definition identifier");
  }
}

std::map<Symbol, Expression*>
Let::get_bindings(Obj li) {
  std::map<Symbol, Expression*> ret {};
  while (is_pair(li)) {
    const auto as_cons = get<Cons*>(li);
    if (!holds_alternative<Cons*>(as_cons->car)) {
      throw runtime_error("Let::get_bindings: type error");
    }
    const auto car = get<Cons*>(as_cons->car);
    if (!holds_alternative<Symbol>(car->car)) {
      throw runtime_error("Let::get_bindings: type error");
    }
    
    ret.insert({
      get<Symbol>(car->at("car")),
      classify(car->at("cadr"))
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
      get<Obj>(p.second->eval(env))
    );
  }
  return ret;
}

Let::Let(Cons *obj):
  Expression("let"s, obj, 3, MAXARGS),
  bindings {get_bindings(obj->at("cadr"))},
  body {combine_expr(obj->at("cddr"))}
{}


bool
Clause::is_else_clause(Obj obj) const {
  return
    holds_alternative<Symbol>(obj) &&
    get<Symbol>(obj).name == "else";
}

Clause::Clause (Cons *obj) {
  if (is_else_clause(obj->car)) {
    is_else = 1;
    predicate = new Literal(true);
  } 
  else {
    is_else = 0;
    predicate = classify(obj->car);
  }
  actions = combine_expr(obj->cdr);
}

If*
Cond::cond2if() const {
  If* ret = new If;
  for (auto curr = clauses.rbegin(); curr != clauses.rend(); curr++) {
    ret = new If(curr->predicate, curr->actions, ret);
  }
  return ret;
}

Cond::Cond(Obj obj):
  Expression("cond"s) 
{
  obj = get<Cons*>(obj)->cdr;
  while (is_pair(obj)) {
    const auto as_cons = get<Cons*>(obj);
    if (!holds_alternative<Cons*>(as_cons->car)) {
      throw runtime_error("cond type error\n");
    }
    const auto new_clause = Clause(get<Cons*>(as_cons->car));
    clauses.push_back(new_clause);
    if (new_clause.is_else)
      break;
    obj = as_cons->cdr;
  }
  if_form = cond2if();
}

Application::Application(Cons *obj):
  Expression("application"s),
  op {classify(obj->car)},
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
  Expression(tag.name, obj, 2, 2),
  word {tag}, 
  expr {classify(obj->at("cadr"))} 
{}

Expression*
combine_expr(Obj seq) {
  const auto vec = cons2exprs(seq);
  Expression *ret;
  if (vec.size() == 0) 
    ret = new Literal(nullptr);
  else if (vec.size() == 1)
    ret = vec[0];
  else
    ret = new Begin(vec);
  return ret;
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

static std::map<Symbol, Expression*(*)(Cons*)> 
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
is_cxr(const string& s) {
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
classify(Obj obj) {
  if (is_pair(obj)) {
    const auto p = get<Cons*>(obj);
    if (holds_alternative<Symbol>(p->car)) {
      const auto tag = get<Symbol>(p->car);
      const auto found = special_forms.find(tag.name);
      if (found != special_forms.end()) {
        const auto func = found->second;
        return func(p);
      }
      else if (is_cxr(tag.name)) {
        return new Cxr(tag, p);
      }
    }
    return new Application(p);
  }
  else if (holds_alternative<Symbol>(obj))
    return new Variable(get<Symbol>(obj));
  else
    return new Literal(obj);
}

}
