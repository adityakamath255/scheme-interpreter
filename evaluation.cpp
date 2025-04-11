#include "common.hpp"
#include "tco.cpp"

namespace Scheme {

Obj
Literal::eval(Environment *env) const {
  return obj;
}

Obj
Variable::eval(Environment *env) const {
  return env->lookup(sym);
}

Obj
Quoted::eval(Environment *env) const {
  return text_of_quotation;
}

Obj
Set::eval(Environment *env) const {
  auto eval_value = value->eval(env);
  env->set_variable(variable, eval_value);
  return eval_value;
}

Obj
If::eval(Environment *env) const {
  if (is_true(predicate->eval(env))) {
    return consequent->eval(env);
  }
  else {
    return alternative->eval(env);
  }
}

Obj
Begin::eval(Environment *env) const {
  Obj ret;
  for (const auto exp : actions) {
    ret = exp->eval(env);
  }
  return ret;
}

Obj
Lambda::eval(Environment *env) const {
  return new Procedure(parameters, body, env);
}


Obj
Define::eval(Environment *env) const {
  env->define_variable(variable, value->eval(env));
  return variable;
}

Obj
Let::eval(Environment *env) const {
  const auto env2 = get_frame(env);
  return body->eval(env2);
}

Obj
Cond::eval(Environment *env) const {
  return if_form->eval(env);
}

Obj
Application::eval(Environment *env) const {
  auto proc = op->eval(env);
  vector<Obj> args {};
  for (const auto param : params) {
    args.push_back(param->eval(env));
  }
  if (at_tail) {
    throw TailCall(proc, args);
  }
  else {
    return apply(proc, args);
  }
} 

Obj
And::eval(Environment *env) const {
  for (const auto exp : exprs) {
    if (is_false(exp->eval(env))) {
      return false;
    }
  }
  return true;
}

Obj
Or::eval(Environment *env) const {
  for (const auto exp : exprs) {
    if (is_true(exp->eval(env))) {
      return true;
    }
  }
  return false;
}

Obj
SetCxr::eval(Environment *env) const {
  auto thing = variable->eval(env);
  if (!holds_alternative<Cons*>(thing)) {
    throw runtime_error("tried to apply set-" + side + "! on a non-pair object");
  }
  const auto edit = value->eval(env);
  get<Cons*>(thing)->at(side) = edit;
  return thing;
}

Obj
Cxr::eval(Environment *env) const {
  const auto val = expr->eval(env);
  if (!holds_alternative<Cons*>(val)) {
    throw runtime_error(word.name + " type error: expected cons");
  }
  auto found = get<Cons*>(val);
  return found->at(word.name);
} 

}
