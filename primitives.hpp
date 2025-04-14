#pragma once
#include "common.hpp"

namespace Scheme {

string stringify(const Obj obj);
vector<std::pair<string, Obj(*)(const vector<Obj>&)>> get_primitive_functions();
vector<std::pair<string, Obj>> get_consts();

}
