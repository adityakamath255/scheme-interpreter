#include "expressions.hpp"

namespace Scheme {

EvalResult
Literal::eval(Environment *env) const {
  return obj;
}

EvalResult
Variable::eval(Environment *env) const {
  return env->lookup(sym);
}

EvalResult
Quoted::eval(Environment *env) const {
  return text_of_quotation;
}

EvalResult
Set::eval(Environment *env) const {
  auto eval_value = get<Obj>(value->eval(env));
  env->set_variable(variable, eval_value);
  return Void {};
}

EvalResult
If::eval(Environment *env) const {
  if (is_true(get<Obj>(predicate->eval(env)))) {
    return consequent->eval(env);
  }
  else {
    return alternative->eval(env);
  }
}

EvalResult
Begin::eval(Environment *env) const {
  for (int i = 0; i < actions.size() - 1; i++) {
    actions[i]->eval(env);
  }
  if (!actions.empty()) {
    return actions.back()->eval(env);
  }
  else {
    return Void {};
  }
}

EvalResult
Lambda::eval(Environment *env) const {
  return new Procedure(parameters, body, env);
}


EvalResult
Define::eval(Environment *env) const {
  env->define_variable(variable, get<Obj>(value->eval(env)));
  return Void {};
}

EvalResult
Let::eval(Environment *env) const {
  const auto env2 = get_frame(env);
  return body->eval(env2);
}

EvalResult
Cond::eval(Environment *env) const {
  return if_form->eval(env);
}

EvalResult
Application::eval(Environment *env) const {
  auto proc = get<Obj>(op->eval(env));
  vector<Obj> args {};
  for (const auto param : params) {
    args.push_back(get<Obj>(param->eval(env)));
  }
  if (at_tail) {
    return TailCall(proc, move(args));
  }
  else {
    return apply(proc, args);
  }
} 

EvalResult
And::eval(Environment *env) const {
  for (const auto exp : exprs) {
    if (is_false(get<Obj>(exp->eval(env)))) {
      return false;
    }
  }
  return true;
}

EvalResult
Or::eval(Environment *env) const {
  for (const auto exp : exprs) {
    if (is_true(get<Obj>(exp->eval(env)))) {
      return true;
    }
  }
  return false;
}

EvalResult
Cxr::eval(Environment *env) const {
  auto val = get<Obj>(expr->eval(env));
  if (!is_pair(val)) {
    throw runtime_error(word.name + " type error: expected cons");
  }
  auto found = get<Cons*>(val);
  return found->at(word.name);
} 

}
