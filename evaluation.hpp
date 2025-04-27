#pragma once
#include "types.hpp"

namespace Scheme {

class Interpreter;

EvalResult apply(Obj, ArgList, Interpreter&);

}
