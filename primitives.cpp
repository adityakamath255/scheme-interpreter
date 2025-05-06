#include "types.hpp"
#include "primitives.hpp"
#include "expressions.hpp"
#include "interpreter.hpp"
#include <functional>

namespace Scheme {

void
PrimitivePutter::put(const std::string& str, const std::function<Obj(const ArgList&, Interpreter&)> func) {
  env->define(interp.intern_symbol(str), interp.spawn<Primitive>(func));
}

void
PrimitivePutter::put_all_functions() {
  put_numeric_functions();
  put_data_functions();
  put_predicates();
  put_misc_functions();
}

}
