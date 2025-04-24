#pragma once
#include "types.hpp"

namespace Scheme {

class Interpreter;

std::vector<std::pair<std::string, Obj(*)(const ArgList&, Interpreter&)>> get_primitive_functions();
std::vector<std::pair<std::string, Obj>> get_consts();

}
