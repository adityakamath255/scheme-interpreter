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
apply(Obj p, ArgList args) {
  while (true) {
    if (is_primitive(p)) {
      const auto func = *as_primitive(p);
      return func(args);
    }
    else if (is_procedure(p)) {
      const auto func = as_procedure(p);
      if (func->parameters.size() != args.size()) {
        throw std::runtime_error(" wrong number of arguments: expected " + std::to_string(func->parameters.size()));
      }
      const auto new_env = func->env->extend(func->parameters, args);
      auto res = func->body->eval(new_env);
      if (is_obj(res)) {
        return res;
      }
      else if (is_tailcall(res)) {
        p = std::move(as_tailcall(res).proc);
        args = std::move(as_tailcall(res).args);
      }
    }
    else {
      throw std::runtime_error("tried to apply an object that is not a procedure");
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
  auto eval_value = as_obj(value->eval(env));
  env->set_variable(variable, eval_value);
  return Void {};
}

EvalResult
If::eval(Environment *env) const {
  if (is_true(as_obj(predicate->eval(env)))) {
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
  env->define_variable(variable, as_obj(value->eval(env)));
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
  auto proc = as_obj(op->eval(env));
  ArgList args {};
  for (const auto& param : params) {
    args.push_back(as_obj(param->eval(env)));
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
  for (const auto& exp : exprs) {
    if (is_false(as_obj(exp->eval(env)))) {
      return false;
    }
  }
  return true;
}

EvalResult
Or::eval(Environment *env) const {
  for (const auto& exp : exprs) {
    if (is_true(as_obj(exp->eval(env)))) {
      return true;
    }
  }
  return false;
}

EvalResult
Cxr::eval(Environment *env) const {
  auto val = as_obj(expr->eval(env));
  if (!is_pair(val)) {
    throw std::runtime_error(word.get_name() + " type error: expected cons");
  }
  auto found = as_pair(val);
  return found->at(word.get_name());
} 

}
