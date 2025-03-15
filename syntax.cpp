#include "common.hpp"
#define MAXARGS 256
using namespace std;

namespace scheme {

struct self_evaluating : public expression {
  sc_obj obj;
  self_evaluating(sc_obj obj):
    expression("self-evaluating"),
    obj {obj}
  {}

  sc_obj
  eval(environment* env) const {
    return obj;
  }
};

// --- // --- //

struct variable : public expression {
  symbol sym;
  variable(symbol& obj):
    expression("variable"),
    sym {obj}
  {}

  sc_obj
  eval(environment* env) const {
    return env->lookup(sym);
  }
};

// --- // --- //

struct quoted : public expression {
  sc_obj text_of_quotation;
  quoted(cons *obj):
    expression("quoted", obj, 2, 2),
    text_of_quotation {obj->at("cadr")}
  {}

  sc_obj
  eval(environment *env) const {
    return text_of_quotation;
  }
};

// --- // --- //

struct assignment : public expression {
  symbol variable;
  expression *value;

  assignment(cons *obj):
    expression("assignment", obj, 3, 3) 
  {
    if (!holds_alternative<symbol>(obj->at("cadr"))) {
      throw runtime_error("tried to assign something to a non-variable");
    }
    variable = get<symbol>(obj->at("cadr"));
    const auto cddr = obj->at("cddr");
    value = classify(obj->at("caddr"));
  }

  sc_obj
  eval(environment *env) const {
    env->set_variable(variable, value->eval(env));
    return "ok";
  }
};

// --- // --- //

struct if_expr : public expression {
  expression *predicate, *consequent, *alternative;

  if_expr(cons *obj):
    expression("if", obj, 3, 4),
    predicate {classify(obj->at("cadr"))},
    consequent {classify(obj->at("caddr"))},
    alternative {
      !is_null(obj->at("cdddr"))
    ? classify(obj->at("cadddr"))
    : new self_evaluating("false")
    }
  {}

  if_expr(expression *p, expression *c, expression *a):
    expression("if"),
    predicate {p},
    consequent {c},
    alternative {a} 
  {}

  if_expr():
    expression("if"),
    predicate {new self_evaluating(false)},
    consequent {new self_evaluating(false)},
    alternative {new self_evaluating(false)} {}

  sc_obj
  eval(environment *env) const {
    if (is_true(predicate->eval(env))) {
      return consequent->eval(env);
    }
    else {
      return alternative->eval(env);
    }
  }
};

// --- // --- //

struct begin_expr : public expression {
public:
  vector<expression*> actions;
  begin_expr(const vector<expression*>& seq):
    expression("begin"),
    actions {seq}
  {}

  sc_obj
  eval(environment *env) const {
    sc_obj ret;
    for (const auto exp : actions) {
      ret = exp->eval(env);
    }
    return ret;
  }
};

expression*
combine_expr(sc_obj seq) {
  const auto vec = cons2vec(seq);
  if (vec.size() == 0) 
    return new self_evaluating(nullptr);
  else if (vec.size() == 1)
    return vec[0];
  else
    return new begin_expr(vec);
}

// --- // --- //

struct lambda_expr : public expression {
  vector<symbol> parameters;
  expression *body;

  lambda_expr(cons *obj):
    expression("lambda", obj, 3, MAXARGS),
    parameters {cons2symbols(obj->at("cadr"))},
    body {combine_expr(obj->at("cddr"))}
  {}

  lambda_expr(sc_obj parameters_, sc_obj body_):
    expression("lambda"),
    parameters {cons2symbols(parameters_)},
    body {combine_expr(body_)}
  {}

  sc_obj
  eval(environment *env) const {
    return new procedure(parameters, body, env);
  }
};

// --- // --- //

struct definition : public expression {
  symbol variable;
  expression *value;

  definition(cons *obj):
    expression("define", obj, 3, MAXARGS)
  {
    const auto cadr = obj->at("cadr");

    if (holds_alternative<symbol>(cadr)) {  
      variable = get<symbol>(cadr);
      value = classify(obj->at("caddr"));
    }

    else if (holds_alternative<cons*>(cadr)) {
      const auto parameters = obj->at("cdadr");
      const auto body = obj->at("cddr");
      if (!holds_alternative<symbol>(obj->at("caadr"))) {
        throw runtime_error("procedure name must be a symbol");
      }
      variable = get<symbol>(obj->at("caadr"));
      value = new lambda_expr(parameters, body);
    }

    else {
      throw runtime_error("bad definition identifier");
    }
  }

  sc_obj
  eval(environment *env) const {
    env->define_variable(variable, value->eval(env));
    return "ok";
  }
};

// --- // --- //

struct let_expr : public expression {
  map<symbol, expression*> bindings;
  expression *body;

  decltype(bindings)
  get_bindings(sc_obj li) {
    map<symbol, expression*> ret {};
    while (is_pair(li)) {
      const auto as_cons = get<cons*>(li);
      if (!holds_alternative<cons*>(as_cons->car)) {
        throw runtime_error("let_expr::get_bindings: type error");
      }
      const auto car = get<cons*>(as_cons->car);
      if (!holds_alternative<symbol>(car->car)) {
        throw runtime_error("let_expr::get_bindings: type error");
      }
      
      ret.insert({
        get<symbol>(car->at("car")),
        classify(car->at("cadr"))
      });
      li = as_cons->cdr;
    }
    return ret;
  }

  environment *
  get_frame(environment *env) const {
    auto ret = new environment (env);
    for (const auto& p : bindings) {
      ret->define_variable(
        p.first,
        p.second->eval(env)
      );
    }
    return ret;
  }

  let_expr(cons *obj):
    expression("let", obj, 3, MAXARGS),
    bindings {get_bindings(obj->at("cadr"))},
    body {combine_expr(obj->at("cddr"))}
  {}

  sc_obj
  eval(environment *env) const {
    const auto env2 = get_frame(env);
    return body->eval(env2);
  }
};

// --- // --- //

struct clause {
private:
  bool
  is_else_clause(sc_obj obj) const {
    return
      holds_alternative<symbol>(obj) &&
      get<symbol>(obj).name == "else";
  };

public:
  bool is_else;
  expression *predicate;
  expression *actions;

  clause (cons *obj) {
    if (is_else_clause(obj->car)) {
      is_else = 1;
      predicate = new self_evaluating(true);
    } else {
      is_else = 0;
      predicate = classify(obj->car);
    }
    actions = combine_expr(obj->cdr);
  }
};

struct cond_expr : public expression {
private:
  if_expr*
  cond2if() const {
    if_expr* ret = new if_expr;
    for (auto curr = clauses.rbegin(); curr != clauses.rend(); curr++) {
      ret = new if_expr(curr->predicate, curr->actions, ret);
    }
    return ret;
  }

public:
  vector<clause> clauses;
  expression *if_form;

  cond_expr (sc_obj obj):
    expression("cond") 
  {
    obj = get<cons*>(obj)->cdr;
    while (is_pair(obj)) {
      const auto as_cons = get<cons*>(obj);
      if (!holds_alternative<cons*>(as_cons->car)) {
        throw runtime_error("cond type error\n");
      }
      const auto new_clause = clause(get<cons*>(as_cons->car));
      clauses.push_back(new_clause);
      if (new_clause.is_else)
        break;
      obj = as_cons->cdr;
    }
    if_form = cond2if();
  }

  sc_obj
  eval(environment *env) const {
    return if_form->eval(env);
  }
};

// --- // --- //

struct application : public expression {
  expression *op;
  vector<expression*> params;

  application(cons *obj):
    expression("application"),
    op {classify(obj->car)},
    params {cons2vec(obj->cdr)}
  {}

  sc_obj
  eval(environment *env) const {
    vector<sc_obj> args {};
    for (const auto param : params) {
      args.push_back(param->eval(env));
    }
    return scheme::apply(op->eval(env), args);
  } 
};

// --- // --- //

struct and_expr : public expression {
  vector<expression*> exprs;

  and_expr(cons *obj):
    expression("and", obj, 2, MAXARGS),
    exprs {cons2vec(obj->cdr)}
  {}

  sc_obj
  eval(environment *env) const {
    for (const auto exp : exprs) {
      if (is_false(exp->eval(env))) {
        return false;
      }
    }
    return true;
  }
};

// --- // --- //

struct or_expr : public expression {
  vector<expression*> exprs;

  or_expr(cons *obj):
    expression("or", obj, 2, MAXARGS),
    exprs {cons2vec(obj->cdr)}
  {}

  sc_obj
  eval(environment *env) const {
    for (const auto exp : exprs) {
      if (is_true(exp->eval(env))) {
        return true;
      }
    }
    return false;
  }
};

// --- // --- //

struct cons_set_expr : public expression {
  expression *variable;
  expression *value;
  string side;

  cons_set_expr(cons *obj, string side): 
    expression("set-" + side + "!", obj, 3, 3),
    variable {classify(obj->at("cadr"))},
    value {classify(obj->at("caddr"))},
    side {side}
  {}

  sc_obj
  eval(environment *env) const {
    auto thing = variable->eval(env);
    if (!holds_alternative<cons*>(thing)) {
      throw runtime_error("tried to apply set-" + side + "! on a non-pair object");
    }
    const auto edit = value->eval(env);
    get<cons*>(thing)->at(side) = edit;
    return thing;
  }
};

// --- // --- //

struct cxr_expr : public expression {
  symbol word;
  expression *expr;

  cxr_expr(symbol tag, cons *obj): 
    expression(tag.name, obj, 2, 2),
    word {tag}, 
    expr {classify(obj->at("cadr"))} 
  {}

  sc_obj
  eval(environment *env) const {
    const auto val = expr->eval(env);
    if (!holds_alternative<cons*>(val)) {
      throw runtime_error(word.name + " type error: expected cons");
    }
    auto found = get<cons*>(val);
    return found->at(word.name);
  } 
};

// --- // --- //

expression*
make_quoted(cons *obj) {
  return new quoted(obj); 
}
expression*
make_assignment(cons *obj) {
  return new assignment(obj); 
}
expression*
make_definition(cons *obj) {
  return new definition(obj);
}
expression*
make_if_expr(cons *obj) {
  return new if_expr(obj);
}
expression*
make_lambda_expr(cons *obj) {
  return new lambda_expr(obj); 
}
expression*
make_let_expr(cons *obj) {
  return new let_expr(obj);
}
expression*
make_begin_expr(cons *obj) {
  return combine_expr(obj->cdr);
}
expression*
make_cond_expr(cons *obj) {
  return new cond_expr(obj); 
}
expression*
make_application(cons *obj) {
  return new application(obj); 
}
expression*
make_and_expr(cons *obj) {
  return new and_expr(obj);
}
expression*
make_or_expr(cons *obj) {
  return new or_expr(obj);
}
expression*
make_set_car_expr(cons *obj) {
  return new cons_set_expr(obj, "car");
}
expression*
make_set_cdr_expr(cons *obj) {
  return new cons_set_expr(obj, "cdr");
}

map<symbol, expression*(*)(cons*)> special_forms = {
  {"quote"s, make_quoted},
  {"set!"s, make_assignment},
  {"define"s, make_definition},
  {"if"s, make_if_expr},
  {"lambda"s, make_lambda_expr},
  {"let"s, make_let_expr},
  {"begin"s, make_begin_expr},
  {"cond"s, make_cond_expr},
  {"and"s, make_and_expr},
  {"or"s, make_or_expr},
  {"set-car!"s, make_set_car_expr},
  {"set-cdr!"s, make_set_cdr_expr}
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

expression*
classify(sc_obj obj) {
  if (is_pair(obj)) {
    const auto p = get<cons*>(obj);
    if (holds_alternative<symbol>(p->car)) {
      const auto tag = get<symbol>(p->car);
      const auto found = special_forms.find(tag.name);
      if (found != special_forms.end()) {
        const auto func = found->second;
        return func(p);
      }
      else if (is_cxr(tag.name)) {
        return new cxr_expr(tag, p);
      }
    }
    return new application(p);
  }
  else if (holds_alternative<symbol>(obj))
    return new variable(get<symbol>(obj));
  else
    return new self_evaluating(obj);
}

}
