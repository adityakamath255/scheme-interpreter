#include "../primitives.hpp"
#include "../expressions.hpp"
#include "../evaluation.hpp"
#include "common.hpp"
#include <iostream>
#include <sstream>

namespace Scheme {

void
PrimitivePutter::put_misc_functions() {
  put("newline", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 0, 0);
    std::cout << std::endl;
    return Void {};
  });

  put("display", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    std::cout << stringify(args[0]);
    return Void {};
  });

  put("error", [](const ArgList& args, Interpreter& interp) -> Obj {
    std::ostringstream message;
    message << "ERROR: ";
    for (auto obj : args) {
      message << stringify(obj);
      message << " ";
    }
    throw std::runtime_error(message.str());
  });
  
  put("eval", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    auto ast = build_ast(args[0], interp);
    return as_obj(ast->eval(interp.get_global_env(), interp));
  });

  put("apply", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_callable(args[0]);
    assert_list(args[1]);
    ArgList apply_args {};
    Obj ls = args[1];
    while (is_pair(ls)) {
      apply_args.push_back(as_pair(ls)->car);
      ls = as_pair(ls)->cdr;
    }
    return as_obj(apply(args[0], std::move(apply_args), interp));
  });

}

};
