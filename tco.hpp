#pragma once 
#include "types.hpp"

namespace Scheme {

struct TailCall {
  Obj proc;
  vector<Obj> args;
  TailCall(Obj proc, vector<Obj> args);
};  

}
