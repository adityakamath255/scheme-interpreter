#pragma once
#include "types.hpp"
#include "expressions.hpp"
#include "environment.hpp"

namespace Scheme {

EvalResult eval(Expression *expr, Environment *const env);
EvalResult apply(Obj p, vector<Obj> args);

}
