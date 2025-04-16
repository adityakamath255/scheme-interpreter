#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "tco.hpp"

namespace Scheme {

EvalResult 
eval(Expression *expr, Environment *const env) {
  return expr->eval(env);
}

EvalResult 
apply(Obj p, vector<Obj> args) {
  while (true) {
    if (is_primitive(p)) {
      const auto func = *get<Primitive*>(p);
      return func(args);
    }
    else if (is_procedure(p)) {
      const auto func = get<Procedure*>(p);
      if (func->parameters.size() != args.size()) {
        throw runtime_error(" wrong number of arguments: expected " + std::to_string(func->parameters.size()));
      }
      const auto new_env = func->env->extend(func->parameters, args);
      auto res = func->body->eval(new_env);
      if (std::holds_alternative<Obj>(res)) {
        return res;
      }
      else if (std::holds_alternative<TailCall>(res)) {
        p = std::move(get<TailCall>(res).proc);
        args = std::move(get<TailCall>(res).args);
      }
    }
    else {
      throw runtime_error("tried to apply an object that is not a procedure");
    }
  } 
}

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
