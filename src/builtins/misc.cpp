#include <builtins/common.hpp>
#include <builtins/installer.hpp>
#include <interpreter/expressions.hpp>
#include <interpreter/evaluation.hpp>
#include <iostream>
#include <sstream>

namespace Scheme {

void
BuiltinInstaller::install_misc_functions() {
  install("newline", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 0, 0);
    std::cout << std::endl;
    return Void {};
  });

  install("display", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    std::cout << stringify(args[0]);
    std::cout.flush();
    return Void {};
  });

  install("error", [](const ArgList& args, Interpreter& interp) -> Obj {
    std::ostringstream message;
    message << "ERROR: ";
    for (auto obj : args) {
      message << stringify(obj);
      message << " ";
    }
    throw std::runtime_error(message.str());
  });
  
  install("eval", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    auto ast = build_ast(args[0], interp);
    return as_obj(ast->eval(interp.get_global_env(), interp));
  });

  install("apply", [](const ArgList& args, Interpreter& interp) {
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
