#include "common.hpp"
#include "tco.cpp"
using namespace std;

namespace scheme {

sc_obj
self_evaluating::eval(environment *env) const {
  return obj;
}

sc_obj
variable::eval(environment *env) const {
  return env->lookup(sym);
}

sc_obj
quoted::eval(environment *env) const {
  return text_of_quotation;
}

sc_obj
assignment::eval(environment *env) const {
  auto eval_value = value->eval(env);
  env->set_variable(variable, eval_value);
  return eval_value;
}

sc_obj
if_expr::eval(environment *env) const {
  if (is_true(predicate->eval(env))) {
    return consequent->eval(env);
  }
  else {
    return alternative->eval(env);
  }
}

sc_obj
begin_expr::eval(environment *env) const {
  sc_obj ret;
  for (const auto exp : actions) {
    ret = exp->eval(env);
  }
  return ret;
}

sc_obj
lambda_expr::eval(environment *env) const {
  return new procedure(parameters, body, env);
}


sc_obj
definition::eval(environment *env) const {
  env->define_variable(variable, value->eval(env));
  return variable;
}

sc_obj
let_expr::eval(environment *env) const {
  const auto env2 = get_frame(env);
  return body->eval(env2);
}

sc_obj
cond_expr::eval(environment *env) const {
  return if_form->eval(env);
}

sc_obj
application::eval(environment *env) const {
  auto proc = op->eval(env);
  vector<sc_obj> args {};
  for (const auto param : params) {
    args.push_back(param->eval(env));
  }
  if (at_tail) {
    throw tail_call(proc, args);
  }
  else {
    return scheme::apply(proc, args);
  }
} 

sc_obj
and_expr::eval(environment *env) const {
  for (const auto exp : exprs) {
    if (is_false(exp->eval(env))) {
      return false;
    }
  }
  return true;
}

sc_obj
or_expr::eval(environment *env) const {
  for (const auto exp : exprs) {
    if (is_true(exp->eval(env))) {
      return true;
    }
  }
  return false;
}

sc_obj
cons_set_expr::eval(environment *env) const {
  auto thing = variable->eval(env);
  if (!holds_alternative<cons*>(thing)) {
    throw runtime_error("tried to apply set-" + side + "! on a non-pair object");
  }
  const auto edit = value->eval(env);
  get<cons*>(thing)->at(side) = edit;
  return thing;
}

sc_obj
cxr_expr::eval(environment *env) const {
  const auto val = expr->eval(env);
  if (!holds_alternative<cons*>(val)) {
    throw runtime_error(word.name + " type error: expected cons");
  }
  auto found = get<cons*>(val);
  return found->at(word.name);
} 

}
