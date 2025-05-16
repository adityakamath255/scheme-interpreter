#include <builtins/installer.hpp>
#include <interpreter/types.hpp>
#include <interpreter/expressions.hpp>
#include <interpreter/interpreter.hpp>
#include <functional>

namespace Scheme {

void
BuiltinInstaller::install(const std::string& str, const std::function<Obj(const ArgList&, Interpreter&)> func) {
  env->define(interp.intern_symbol(str), interp.spawn<Builtin>(func));
}

void
BuiltinInstaller::install_all_functions() {
  install_numeric_functions();
  install_data_functions();
  install_predicates();
  install_misc_functions();
}

}
