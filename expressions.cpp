#include "common.hpp"

constexpr int MAXARGS = 256;

namespace Scheme {

struct Literal : public Expression {
  Obj obj;
  Literal(Obj obj):
    Expression("self-evaluating"),
    obj {obj}
  {}

  Obj eval(Environment* env) const override;
};

// --- // --- //

struct Variable : public Expression {
  Symbol sym;
  Variable(Symbol& obj):
    Expression("Variable"),
    sym {obj}
  {}

  Obj eval(Environment* env) const override;
};

// --- // --- //

struct Quoted : public Expression {
  Obj text_of_quotation;
  Quoted(Cons *obj):
    Expression("quoted", obj, 2, 2),
    text_of_quotation {obj->at("cadr")}
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct Set : public Expression {
  Symbol variable;
  Expression *value;

  Set(Cons *obj):
    Expression("Set", obj, 3, 3)
  {
    if (!holds_alternative<Symbol>(obj->at("cadr"))) {
      throw runtime_error("tried to assign something to a non-variable");
    }
    variable = get<Symbol>(obj->at("cadr"));
    const auto cddr = obj->at("cddr");
    value = classify(obj->at("caddr"));
  }

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct If : public Expression {
  Expression *predicate, *consequent, *alternative;

  If(Cons *obj):
    Expression("if", obj, 3, 4),
    predicate {classify(obj->at("cadr"))},
    consequent {classify(obj->at("caddr"))},
    alternative {
      !is_null(obj->at("cdddr"))
    ? classify(obj->at("cadddr"))
    : new Literal("false")
    }
  {}

  If(Expression *p, Expression *c, Expression *a):
    Expression("if"),
    predicate {p},
    consequent {c},
    alternative {a} 
  {}

  If():
    Expression("if"),
    predicate {new Literal(false)},
    consequent {new Literal(false)},
    alternative {new Literal(false)} {}

  Obj eval(Environment *env) const override;
  void tco() override;
};

// --- // --- //

struct Begin : public Expression {
public:
  vector<Expression*> actions;
  Begin(const vector<Expression*>& seq):
    Expression("begin"),
    actions {seq}
  {}

  Obj eval(Environment *env) const override;
  void tco() override;
};

Expression*
combine_expr(Obj seq) {
  const auto vec = cons2vec(seq);
  Expression *ret;
  if (vec.size() == 0) 
    ret = new Literal(nullptr);
  else if (vec.size() == 1)
    ret = vec[0];
  else
    ret = new Begin(vec);
  ret->tco();
  return ret;
}

// --- // --- //

struct Lambda : public Expression {
  vector<Symbol> parameters;
  Expression *body;

  Lambda(Cons *obj):
    Expression("lambda", obj, 3, MAXARGS),
    parameters {cons2symbols(obj->at("cadr"))},
    body {combine_expr(obj->at("cddr"))}
  {}

  Lambda(Obj parameters_, Obj body_):
    Expression("lambda"),
    parameters {cons2symbols(parameters_)},
    body {combine_expr(body_)}
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct Define : public Expression {
  Symbol variable;
  Expression *value;

  Define(Cons *obj):
    Expression("define", obj, 3, MAXARGS)
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

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct Let : public Expression {
  map<Symbol, Expression*> bindings;
  Expression *body;

  decltype(bindings)
  get_bindings(Obj li) {
    map<Symbol, Expression*> ret {};
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
  get_frame(Environment *env) const {
    auto ret = new Environment (env);
    for (const auto& p : bindings) {
      ret->define_variable(
        p.first,
        p.second->eval(env)
      );
    }
    return ret;
  }

  Let(Cons *obj):
    Expression("let", obj, 3, MAXARGS),
    bindings {get_bindings(obj->at("cadr"))},
    body {combine_expr(obj->at("cddr"))}
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct clause {
private:
  bool
  is_else_clause(Obj obj) const {
    return
      holds_alternative<Symbol>(obj) &&
      get<Symbol>(obj).name == "else";
  };

public:
  bool is_else;
  Expression *predicate;
  Expression *actions;

  clause (Cons *obj) {
    if (is_else_clause(obj->car)) {
      is_else = 1;
      predicate = new Literal(true);
    } else {
      is_else = 0;
      predicate = classify(obj->car);
    }
    actions = combine_expr(obj->cdr);
  }
};

struct Cond : public Expression {
private:
  If*
  cond2if() const {
    If* ret = new If;
    for (auto curr = clauses.rbegin(); curr != clauses.rend(); curr++) {
      ret = new If(curr->predicate, curr->actions, ret);
    }
    return ret;
  }

public:
  vector<clause> clauses;
  Expression *if_form;

  Cond (Obj obj):
    Expression("cond") 
  {
    obj = get<Cons*>(obj)->cdr;
    while (is_pair(obj)) {
      const auto as_cons = get<Cons*>(obj);
      if (!holds_alternative<Cons*>(as_cons->car)) {
        throw runtime_error("cond type error\n");
      }
      const auto new_clause = clause(get<Cons*>(as_cons->car));
      clauses.push_back(new_clause);
      if (new_clause.is_else)
        break;
      obj = as_cons->cdr;
    }
    if_form = cond2if();
  }

  Obj eval(Environment *env) const override;
  void tco() override;
};

// --- // --- //

struct Application : public Expression {
  Expression *op;
  vector<Expression*> params;
  bool at_tail = false;

  Application(Cons *obj):
    Expression("Application"),
    op {classify(obj->car)},
    params {cons2vec(obj->cdr)}
  {}

  Obj eval(Environment *env) const override;
  void tco() override;
};

// --- // --- //

struct And : public Expression {
  vector<Expression*> exprs;

  And(Cons *obj):
    Expression("and", obj, 2, MAXARGS),
    exprs {cons2vec(obj->cdr)}
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct Or : public Expression {
  vector<Expression*> exprs;

  Or(Cons *obj):
    Expression("or", obj, 2, MAXARGS),
    exprs {cons2vec(obj->cdr)}
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct SetCxr : public Expression {
  Expression *variable;
  Expression *value;
  string side;

  SetCxr(Cons *obj, string side): 
    Expression("set-" + side + "!", obj, 3, 3),
    variable {classify(obj->at("cadr"))},
    value {classify(obj->at("caddr"))},
    side {side}
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

struct Cxr : public Expression {
  Symbol word;
  Expression *expr;

  Cxr(Symbol tag, Cons *obj): 
    Expression(tag.name, obj, 2, 2),
    word {tag}, 
    expr {classify(obj->at("cadr"))} 
  {}

  Obj eval(Environment *env) const override;
};

// --- // --- //

Expression* make_quoted(Cons *obj) { return new Quoted(obj); }
Expression* make_set(Cons *obj) { return new Set(obj); }
Expression* make_define(Cons *obj) { return new Define(obj); }
Expression* make_if(Cons *obj) { return new If(obj); }
Expression* make_lambda(Cons *obj) { return new Lambda(obj); }
Expression* make_let(Cons *obj) { return new Let(obj); }
Expression* make_begin(Cons *obj) { return combine_expr(obj->cdr); }
Expression* make_cond(Cons *obj) { return new Cond(obj); }
Expression* make_application(Cons *obj) { return new Application(obj); }
Expression* make_and(Cons *obj) { return new And(obj); }
Expression* make_or(Cons *obj) { return new Or(obj); }
Expression* make_set_car(Cons *obj) { return new SetCxr(obj, "car"); }
Expression* make_set_cdr(Cons *obj) { return new SetCxr(obj, "cdr"); }

map<Symbol, Expression*(*)(Cons*)> special_forms = {
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
  {"set-car!"s, make_set_car},
  {"set-cdr!"s, make_set_cdr}
};

bool
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
