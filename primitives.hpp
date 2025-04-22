#pragma once
#include "types.hpp"

namespace Scheme {

std::vector<std::pair<std::string, Obj(*)(const ArgList&)>> get_primitive_functions();
std::vector<std::pair<std::string, Obj>> get_consts();

}
