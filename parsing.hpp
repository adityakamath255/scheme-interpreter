#pragma once
#include "types.hpp"
#include <utility>

namespace Scheme {

vector<string> tokenize(const string&);
Obj parse(const vector<string>&);

}
