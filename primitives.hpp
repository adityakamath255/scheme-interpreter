#pragma once
#include "types.hpp"

namespace Scheme {

class Interpreter;

std::vector<std::pair<std::string, Obj(*)(const ArgList&, Interpreter&)>> get_primitive_functions();
std::vector<std::pair<std::string, Obj>> get_consts();

void install_primitives(Environment *env, Interpreter& interp);
void install_consts(Environment *env, Interpreter& interp);

}
