#pragma once 
#include "types.hpp"

namespace Scheme {

struct TailCall {
  Obj proc;
  ArgList args;
  TailCall(Obj, ArgList);
};  

}
