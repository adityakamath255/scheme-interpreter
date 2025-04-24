#pragma once
#include "types.hpp"
#include "expressions.hpp"
#include "environment.hpp"

namespace Scheme {

class Interpreter;

EvalResult apply(Obj, ArgList, Interpreter&);

}
