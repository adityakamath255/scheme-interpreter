#include "interface.hpp"
#include "execution.cpp"
using namespace std;

namespace scheme {

struct self_evaluating : public expression {
  const sc_obj obj;
  self_evaluating(sc_obj obj_):
    obj {obj_}
  {}

  executor*
  analyze() const {
    return new self_eval_exec(obj);
  }
};

struct variable : public expression {
  const symbol sym;
  variable(symbol& obj_):
    sym {obj_}
  {}

  executor*
  analyze() const {
    return new var_exec(sym);
  }
};

struct quoted : public expression {
  const sc_obj text_of_quotation;
  quoted(cons *obj_):
    text_of_quotation {obj_->at("cadr")}
  {}

  executor*
  analyze() const {
    return new self_eval_exec(text_of_quotation);
  }
};

struct assignment : public expression {
  const symbol variable;
  const expression *value;
  assignment(cons *obj_):
    variable {get<symbol>(obj_->at("cadr"))},
    value {classify(obj_->at("caddr"))}
  {}

  executor*
  analyze() const {
    return new set_exec(variable, value->analyze());
  }
};

struct if_expr : public expression {
  const expression *predicate, *consequent, *alternative;

  if_expr(cons *obj_):
    predicate {classify(obj_->at("cadr"))},
    consequent {classify(obj_->at("caddr"))},
    alternative {
      !is_null(obj_->at("cdddr"))
    ? classify(obj_->at("cadddr"))
    : new self_evaluating("false")
    }
  {}

  if_expr(expression *p, expression *c, expression *a):
    predicate {p},
    consequent {c},
    alternative {a} {}

  if_expr():
    predicate {new self_evaluating(false)},
    consequent {new self_evaluating(false)},
    alternative {new self_evaluating(false)} {}

  executor*
  analyze() const {
    return new if_exec(
      predicate->analyze(),
      consequent->analyze(),
      alternative->analyze()
    );
  }
};

struct begin_expr : public expression {
public:
  const vector<expression*> actions;
  begin_expr(const vector<expression*>& seq):
    actions {seq}
  {}

  executor*
  analyze() const {
    vector<executor*> execs {};
    for (const auto expr : actions) {
      execs.push_back(expr->analyze());
    }
    return new begin_exec(execs);
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

struct lambda_expr : public expression {
  const vector<symbol> parameters;
  const expression *body;

  lambda_expr(cons *obj_):
    parameters {cons2symbols(obj_->at("cadr"))},
    body {combine_expr(obj_->at("cddr"))}
  {}

  lambda_expr(sc_obj parameters_, sc_obj body_):
    parameters {cons2symbols(parameters_)},
    body {combine_expr(body_)}
  {}

  executor*
  analyze() const {
    return new lambda_exec(parameters, body->analyze());
  }
};

struct definition : public expression {
  const symbol variable;
  const expression *value;

  definition(symbol v1, expression *v2):
    variable {v1},
    value {v2}
  {}

  executor*
  analyze() const {
    return new def_exec(variable, value->analyze());
  }
};

struct let_expr : public expression {
  const map<symbol, expression*> bindings;
  const expression *body;

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

  map<symbol, executor*> 
  get_pseudoframe() const {
    map<symbol, executor*> pseudoframe {};
    for (const auto& [sym, expr] : bindings)
      pseudoframe.insert({sym, expr->analyze()});
    return pseudoframe;
  }

  let_expr(cons *obj_):
    bindings {get_bindings(obj_->at("cadr"))},
    body {combine_expr(obj_->at("cddr"))}
  {}

  executor*
  analyze() const {
    return new let_exec(get_pseudoframe(), body->analyze());
  }

};

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

  clause (cons *obj_) {
    if (is_else_clause(obj_->car)) {
      is_else = 1;
      predicate = new self_evaluating(true);
    } else {
      is_else = 0;
      predicate = classify(obj_->car);
    }
    actions = combine_expr(obj_->cdr);
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

  cond_expr (sc_obj obj_) {
    obj_ = get<cons*>(obj_)->cdr;
    while (is_pair(obj_)) {
      const auto as_cons = get<cons*>(obj_);
      if (!holds_alternative<cons*>(as_cons->car)) {
        throw runtime_error("cond_expr::cond_expr: type error\n");
      }
      const auto new_clause = clause(get<cons*>(as_cons->car));
      clauses.push_back(new_clause);
      if (new_clause.is_else)
        break;
      obj_ = as_cons->cdr;
    }
  }

  executor*
  analyze() const {
    return cond2if()->analyze();
  }
};

struct application : public expression {
  const expression *op;
  const vector<expression*> args;

  application(cons *obj_):
    op {classify(obj_->car)},
    args {cons2vec(obj_->cdr)}
  {}

  executor*
  analyze() const {
    return new apply_exec(op->analyze(), exprs2execs(args));
  }
};

struct and_expr : public expression {
  const vector<expression*> exprs;

  and_expr(cons *obj_):
    exprs {cons2vec(obj_->cdr)}
  {}

  executor*
  analyze() const {
    return new and_exec(exprs2execs(exprs));
  }
};

struct or_expr : public expression {
  const vector<expression*> exprs;

  or_expr(cons *obj_):
    exprs {cons2vec(obj_->cdr)}
  {}

  executor*
  analyze() const {
    return new or_exec(exprs2execs(exprs));
  }
};

struct cons_set_expr : public expression {
  symbol variable;
  const expression *value;
  const bool side;

  cons_set_expr(cons *obj_, bool b): 
    value {classify(obj_->at("caddr"))},
    side {b}
  {
    if (holds_alternative<symbol>(obj_->at("cadr"))) {
      variable = get<symbol>(obj_->at("cadr"));
    }
    else {
      throw runtime_error("cons_set_expr::cons_set_expr: type error");
    }
  }

  executor*
  analyze() const {
    return new cons_set_exec(variable, value->analyze(), side);
  }
};

struct cxr_expr : public expression {
  symbol word;
  expression *expr;

  cxr_expr(const symbol& s, sc_obj obj_): word {s}, expr {classify(obj_)} {}

  executor*
  analyze() const {
    return new cxr_exec(word.name, expr->analyze());
  }
};

expression*
make_quoted(cons *obj_) {
  return new quoted(obj_); 
}
expression*
make_assignment(cons *obj_) {
  return new assignment(obj_); 
}
expression*
make_definition(cons *obj_) {
  const auto cadr = obj_->at("cadr");

  if (holds_alternative<symbol>(cadr)) {  
    return new definition(
      get<symbol>(cadr),
      classify(obj_->at("caddr"))
    );
  }

  else {
    const auto parameters = obj_->at("cdadr");
    const auto body = obj_->at("cddr");
    if (!holds_alternative<symbol>(obj_->at("caadr"))) {
      throw runtime_error("make_definition: type error\n");
    }
    return new definition(
      get<symbol>(obj_->at("caadr")),
      new lambda_expr(parameters, body)
    );
  }
}

expression*
make_if_expr(cons *obj_) {
  return new if_expr(obj_);
}
expression*
make_lambda_expr(cons *obj_) {
  return new lambda_expr(obj_); 
}
expression*
make_let_expr(cons *obj_) {
  return new let_expr(obj_);
}
expression*
make_begin_expr(cons *obj_) {
  return combine_expr(obj_->cdr);
}
expression*
make_cond_expr(cons *obj_) {
  return new cond_expr(obj_); 
}
expression*
make_application(cons *obj_) {
  return new application(obj_); 
}
expression*
make_and_expr(cons *obj_) {
  return new and_expr(obj_);
}
expression*
make_or_expr(cons *obj_) {
  return new or_expr(obj_);
}
expression*
make_set_car_expr(cons *obj_) {
  return new cons_set_expr(obj_, 0);
}
expression*
make_set_cdr_expr(cons *obj_) {
  return new cons_set_expr(obj_, 1);
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
  for (int i = 1; i < s.size() -1; i++) {
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
        return new cxr_expr(tag, p->at("cadr"));
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