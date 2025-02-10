#include "common.hpp"
#include "execution.cpp"
#define MAXARGS 256
using namespace std;

namespace scheme {

struct self_evaluating : public expression {
  sc_obj obj;
  self_evaluating(sc_obj obj):
    expression("self-evaluating"),
    obj {obj}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<self_eval_exec>(obj);
  }
};

struct variable : public expression {
  symbol sym;
  variable(symbol& obj):
    expression("variable"),
    sym {obj}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<var_exec>(sym);
  }
};

struct quoted : public expression {
  sc_obj text_of_quotation;
  quoted(shared_ptr<cons> obj):
    expression("quoted", obj, 2, 2),
    text_of_quotation {obj->at("cadr")}
  {} 

  shared_ptr<executor> 
  analyze() const {
    return make_shared<self_eval_exec>(text_of_quotation);
  }
};

struct assignment : public expression {
  symbol variable;
  shared_ptr<expression> value;

  assignment(shared_ptr<cons> obj):
    expression("assignment", obj, 3, 3) 
  {
    if (!holds_alternative<symbol>(obj->at("cadr"))) {
      throw runtime_error("tried to assign something to a non-variable");
    }
    variable = get<symbol>(obj->at("cadr"));
    const auto cddr = obj->at("cddr");
    value = classify(obj->at("caddr"));
  }

  shared_ptr<executor> 
  analyze() const {
    return make_shared<set_exec>(variable, value->analyze());
  }
};

struct if_expr : public expression {
  shared_ptr<expression> predicate, consequent, alternative;

  if_expr(shared_ptr<cons> obj):
    expression("if", obj, 3, 4),
    predicate {classify(obj->at("cadr"))},
    consequent {classify(obj->at("caddr"))},
    alternative {
      !is_null(obj->at("cdddr"))
    ? classify(obj->at("cadddr"))
    : make_shared<self_evaluating>("false")
    }
  {}

  if_expr(shared_ptr<expression> p, shared_ptr<expression> c, shared_ptr<expression> a):
    expression("if"),
    predicate {p},
    consequent {c},
    alternative {a} 
  {}

  if_expr():
    expression("if"),
    predicate {make_shared<self_evaluating>(false)},
    consequent {make_shared<self_evaluating>(false)},
    alternative {make_shared<self_evaluating>(false)} {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<if_exec>(
      predicate->analyze(),
      consequent->analyze(),
      alternative->analyze()
    );
  }
};

struct begin_expr : public expression {
public:
  vector<shared_ptr<expression> > actions;
  begin_expr(const vector<shared_ptr<expression> >& seq):
    expression("begin"),
    actions {seq}
  {}

  shared_ptr<executor> 
  analyze() const {
    vector<shared_ptr<executor> > execs {};
    for (const auto expr : actions) {
      execs.push_back(expr->analyze());
    }
    return make_shared<begin_exec>(execs);
  }
};

shared_ptr<expression> 
combine_expr(sc_obj seq) {
  const auto vec = cons2vec(seq);
  if (vec.size() == 0) 
    return make_shared<self_evaluating>(nullptr);
  else if (vec.size() == 1)
    return vec[0];
  else
    return make_shared<begin_expr>(vec);
}

struct lambda_expr : public expression {
  vector<symbol> parameters;
  shared_ptr<expression> body;

  lambda_expr(shared_ptr<cons> obj):
    expression("lambda", obj, 3, MAXARGS),
    parameters {cons2symbols(obj->at("cadr"))},
    body {combine_expr(obj->at("cddr"))}
  {}

  lambda_expr(sc_obj parameters_, sc_obj body_):
    expression("lambda"),
    parameters {cons2symbols(parameters_)},
    body {combine_expr(body_)}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<lambda_exec>(parameters, body->analyze());
  }
};

struct definition : public expression {
  symbol variable;
  shared_ptr<expression> value;

  definition(shared_ptr<cons> obj):
    expression("define", obj, 3, MAXARGS)
  {
    const auto cadr = obj->at("cadr");

    if (holds_alternative<symbol>(cadr)) {  
      variable = get<symbol>(cadr);
      value = classify(obj->at("caddr"));
    }

    else if (holds_alternative<shared_ptr<cons> >(cadr)) {
      const auto parameters = obj->at("cdadr");
      const auto body = obj->at("cddr");
      if (!holds_alternative<symbol>(obj->at("caadr"))) {
        throw runtime_error("procedure name must be a symbol");
      }
      variable = get<symbol>(obj->at("caadr"));
      value = make_shared<lambda_expr>(parameters, body);
    }

    else {
      throw runtime_error("bad definition identifier");
    }
  }

  shared_ptr<executor> 
  analyze() const {
    return make_shared<def_exec>(variable, value->analyze());
  }
};

struct let_expr : public expression {
  map<symbol, shared_ptr<expression> > bindings;
  shared_ptr<expression> body;

  decltype(bindings)
  get_bindings(sc_obj li) {
    map<symbol, shared_ptr<expression> > ret {};
    while (is_pair(li)) {
      const auto as_cons = get<shared_ptr<cons> >(li);
      if (!holds_alternative<shared_ptr<cons> >(as_cons->car)) {
        throw runtime_error("let_expr::get_bindings: type error");
      }
      const auto car = get<shared_ptr<cons> >(as_cons->car);
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

  map<symbol, shared_ptr<executor> > 
  get_pseudoframe() const {
    map<symbol, shared_ptr<executor> > pseudoframe {};
    for (const auto& [sym, expr] : bindings)
      pseudoframe.insert({sym, expr->analyze()});
    return pseudoframe;
  }

  let_expr(shared_ptr<cons> obj):
    expression("let", obj, 3, MAXARGS),
    bindings {get_bindings(obj->at("cadr"))},
    body {combine_expr(obj->at("cddr"))}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<let_exec>(get_pseudoframe(), body->analyze());
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
  shared_ptr<expression> predicate;
  shared_ptr<expression> actions;

  clause (shared_ptr<cons> obj) {
    if (is_else_clause(obj->car)) {
      is_else = 1;
      predicate = make_shared<self_evaluating>(true);
    } else {
      is_else = 0;
      predicate = classify(obj->car);
    }
    actions = combine_expr(obj->cdr);
  }
};

struct cond_expr : public expression {
private:
  shared_ptr<if_expr> 
  cond2if() const {
    auto ret = make_shared<if_expr>();
    for (auto curr = clauses.rbegin(); curr != clauses.rend(); curr++) {
      ret = make_shared<if_expr>(curr->predicate, curr->actions, ret);
    }
    return ret;
  }

public:
  vector<clause> clauses;

  cond_expr (sc_obj obj):
    expression("cond") 
  {
    obj = get<shared_ptr<cons> >(obj)->cdr;
    while (is_pair(obj)) {
      const auto as_cons = get<shared_ptr<cons> >(obj);
      if (!holds_alternative<shared_ptr<cons> >(as_cons->car)) {
        throw runtime_error("cond type error\n");
      }
      const auto new_clause = clause(get<shared_ptr<cons> >(as_cons->car));
      clauses.push_back(new_clause);
      if (new_clause.is_else)
        break;
      obj = as_cons->cdr;
    }
  }

  shared_ptr<executor> 
  analyze() const {
    return cond2if()->analyze();
  }
};

struct application : public expression {
  shared_ptr<expression> op;
  vector<shared_ptr<expression> > args;

  application(shared_ptr<cons> obj):
    expression("application"),
    op {classify(obj->car)},
    args {cons2vec(obj->cdr)}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<apply_exec>(op->analyze(), exprs2execs(args));
  }
};

struct and_expr : public expression {
  vector<shared_ptr<expression> > exprs;

  and_expr(shared_ptr<cons> obj):
    expression("and", obj, 2, MAXARGS),
    exprs {cons2vec(obj->cdr)}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<and_exec>(exprs2execs(exprs));
  }
};

struct or_expr : public expression {
  vector<shared_ptr<expression> > exprs;

  or_expr(shared_ptr<cons> obj):
    expression("or", obj, 2, MAXARGS),
    exprs {cons2vec(obj->cdr)}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<or_exec>(exprs2execs(exprs));
  }
};

struct cons_set_expr : public expression {
  shared_ptr<expression> variable;
  shared_ptr<expression> value;
  string side;

  cons_set_expr(shared_ptr<cons> obj, string side): 
    expression("set-" + side + "!", obj, 3, 3),
    variable {classify(obj->at("cadr"))},
    value {classify(obj->at("caddr"))},
    side {side}
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<cons_set_exec>(variable->analyze(), value->analyze(), side);
  }
};

struct cxr_expr : public expression {
  symbol word;
  shared_ptr<expression> expr;

  cxr_expr(symbol tag, shared_ptr<cons> obj): 
    expression(tag.name, obj, 2, 2),
    word {tag}, 
    expr {classify(obj->at("cadr"))} 
  {}

  shared_ptr<executor> 
  analyze() const {
    return make_shared<cxr_exec>(word.name, expr->analyze());
  }
};

shared_ptr<expression>
make_quoted(shared_ptr<cons> obj) {
  return std::make_shared<quoted>(obj); 
}
shared_ptr<expression>
make_assignment(shared_ptr<cons> obj) {
  return std::make_shared<assignment>(obj); 
}
shared_ptr<expression>
make_definition(shared_ptr<cons> obj) {
  return std::make_shared<definition>(obj);
}
shared_ptr<expression>
make_if_expr(shared_ptr<cons> obj) {
  return std::make_shared<if_expr>(obj);
}
shared_ptr<expression>
make_lambda_expr(shared_ptr<cons> obj) {
  return std::make_shared<lambda_expr>(obj); 
}
shared_ptr<expression>
make_let_expr(shared_ptr<cons> obj) {
  return std::make_shared<let_expr>(obj);
}
shared_ptr<expression>
make_begin_expr(shared_ptr<cons> obj) {
  return combine_expr(obj->cdr);
}
shared_ptr<expression>
make_cond_expr(shared_ptr<cons> obj) {
  return std::make_shared<cond_expr>(obj); 
}
shared_ptr<expression>
make_application(shared_ptr<cons> obj) {
  return std::make_shared<application>(obj); 
}
shared_ptr<expression>
make_and_expr(shared_ptr<cons> obj) {
  return std::make_shared<and_expr>(obj);
}
shared_ptr<expression>
make_or_expr(shared_ptr<cons> obj) {
  return std::make_shared<or_expr>(obj);
}
shared_ptr<expression>
make_set_car_expr(shared_ptr<cons> obj) {
  return std::make_shared<cons_set_expr>(obj, "car");
}
shared_ptr<expression>
make_set_cdr_expr(shared_ptr<cons> obj) {
  return std::make_shared<cons_set_expr>(obj, "cdr");
}

map<symbol, shared_ptr<expression>(*)(shared_ptr<cons>)> special_forms = {
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

shared_ptr<expression> 
classify(sc_obj obj) {
  if (is_pair(obj)) {
    const auto p = get<shared_ptr<cons>>(obj);
    if (holds_alternative<symbol>(p->car)) {
      const auto tag = get<symbol>(p->car);
      const auto found = special_forms.find(tag.name);
      if (found != special_forms.end()) {
        const auto func = found->second;
        return func(p);
      }
      else if (is_cxr(tag.name)) {
        return make_shared<cxr_expr>(tag, p);
      }
    }
    return make_shared<application>(p);
  }
  else if (holds_alternative<symbol>(obj))
    return make_shared<variable>(get<symbol>(obj));
  else
    return make_shared<self_evaluating>(obj);
}

}
