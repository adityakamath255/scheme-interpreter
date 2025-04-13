#pragma once
#include "common.hpp"
#include <utility>

namespace Scheme {

vector<string> tokenize(const string& input);
Obj parse(const vector<string>& tokens);

}
