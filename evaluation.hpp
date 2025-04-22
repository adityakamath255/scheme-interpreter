#pragma once
#include "types.hpp"
#include "expressions.hpp"
#include "environment.hpp"

namespace Scheme {

EvalResult eval(Expression*, Environment *const);
EvalResult apply(Obj, ArgList);

}
