#pragma once
#include <core/types.hpp>
#include <core/expressions.hpp>

namespace Scheme {

class Interpreter;

EvalResult apply(Obj, ArgList, Interpreter&);

}
