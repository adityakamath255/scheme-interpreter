#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "interpreter.hpp"

namespace Scheme {

EvalResult 
eval(Expression *expr, Environment *const env, Interpreter& interp) {
  return expr->eval(env, interp);
}

ArgList
to_variadic_args(ArgList args, const size_t size, Interpreter& interp) {
  args.push_back(nullptr);
  while (args.size() > size) {
    auto& last = args.back();
    auto& second_last = args[args.size() - 2]; 
    second_last = interp.alloc.make<Cons>(second_last, last);
    args.pop_back();
  }
  return args;
}

EvalResult 
apply(Obj p, ArgList args, Interpreter& interp) {
  while (true) {
    if (is_primitive(p)) {
      const auto func = *as_primitive(p);
      return func(args, interp);
    }

    else if (is_procedure(p)) {
      auto func = as_procedure(p);

      if (func->is_variadic) {
        if (args.size() + 1 < func->parameters.size()) {
          throw std::runtime_error(std::format(
            "wrong number of arguments: expected {}+",
            func->parameters.size() - 1
          ));
        }
        args = to_variadic_args(std::move(args), func->parameters.size(), interp);
      }

      else {
        if (args.size() != func->parameters.size()) {
          throw std::runtime_error(std::format(
            "wrong number of arguments: expected {}",
            func->parameters.size()
          ));
        }
      }

      auto new_env = func->env->extend(func->parameters, args, interp);
      auto res = func->body->eval(new_env, interp);
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
Literal::eval(Environment *env, Interpreter& interp) {
  return obj;
}

EvalResult
Variable::eval(Environment *env, Interpreter& interp) {
  if (resolved) {
    for (size_t i = 0; i < depth; i++) {
      env = env->super;
    }
    return env->get(sym);
  }
  else {
    const auto found = env->get_with_depth(sym);
    depth = found.second;
    resolved = true;
    return found.first;
  }
}

EvalResult
Quoted::eval(Environment *env, Interpreter& interp) {
  return text_of_quotation;
}

EvalResult
Set::eval(Environment *env, Interpreter& interp) {
  auto eval_value = as_obj(value->eval(env, interp));
  env->set(variable, eval_value);
  return Void {};
}

EvalResult
If::eval(Environment *env, Interpreter& interp) {
  if (is_true(as_obj(predicate->eval(env, interp)))) {
    return consequent->eval(env, interp);
  }
  else {
    return alternative->eval(env, interp);
  }
}

EvalResult
Begin::eval(Environment *env, Interpreter& interp) {
  for (size_t i = 0; i < actions.size() - 1; i++) {
    actions[i]->eval(env, interp);
  }
  if (!actions.empty()) {
    return actions.back()->eval(env, interp);
  }
  else {
    return Void {};
  }
}

EvalResult
Lambda::eval(Environment *env, Interpreter& interp) {
  return interp.alloc.make<Procedure>(parameters, body, env, is_variadic);
}


EvalResult
Define::eval(Environment *env, Interpreter& interp) {
  env->define(variable, as_obj(value->eval(env, interp)));
  return Void {};
}

static Environment *
get_frame(const Let& expr, Environment *env, Interpreter& interp) {
  auto ret = env->extend(interp);
  for (const auto& p : expr.bindings) {
    ret->define(
      p.first,
      as_obj(p.second->eval(env, interp))
    );
  }
  return ret;
}

EvalResult
Let::eval(Environment *env, Interpreter& interp) {
  const auto env2 = get_frame(*this, env, interp);
  return body->eval(env2, interp);
}

EvalResult
Cond::eval(Environment *env, Interpreter& interp) {
  for (const Clause& clause : clauses) {
    if (clause.is_else) {
      return clause.actions->eval(env, interp);
    }
    else {
      auto pred_output = as_obj(clause.predicate->eval(env, interp));
      if (is_true(pred_output)) {
        if (clause.has_actions) {
          return clause.actions->eval(env, interp);
        }
        else {
          return pred_output;
        }
      }
    }
  }
  return Void {}; 
}

EvalResult
Application::eval(Environment *env, Interpreter& interp) {
  auto proc = as_obj(op->eval(env, interp));
  ArgList args {};
  for (const auto& param : params) {
    args.push_back(as_obj(param->eval(env, interp)));
  }
  if (at_tail) {
    return TailCall(proc, move(args));
  }
  else {
    return apply(proc, args, interp);
  }
} 

EvalResult
And::eval(Environment *env, Interpreter& interp) {
  for (const auto& exp : exprs) {
    if (is_false(as_obj(exp->eval(env, interp)))) {
      return false;
    }
  }
  return true;
}

EvalResult
Or::eval(Environment *env, Interpreter& interp) {
  for (const auto& exp : exprs) {
    if (is_true(as_obj(exp->eval(env, interp)))) {
      return true;
    }
  }
  return false;
}

EvalResult
Cxr::eval(Environment *env, Interpreter& interp) {
  auto val = as_obj(expr->eval(env, interp));
  if (!is_pair(val)) {
    throw std::runtime_error(word.get_name() + " type error: expected cons");
  }
  auto found = as_pair(val);
  return found->at(word.get_name());
} 

}
