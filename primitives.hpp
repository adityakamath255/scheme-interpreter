#pragma once
#include "types.hpp"

namespace Scheme {

vector<std::pair<string, Obj(*)(const vector<Obj>&)>> get_primitive_functions();
vector<std::pair<string, Obj>> get_consts();

}
