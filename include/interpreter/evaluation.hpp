#pragma once
#include <interpreter/types.hpp>
#include <interpreter/expressions.hpp>

namespace Scheme {

class Interpreter;

EvalResult apply(Obj, ArgList, Interpreter&);

}
