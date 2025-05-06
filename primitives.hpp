#pragma once
#include "types.hpp"

namespace Scheme {

class Interpreter;

class PrimitivePutter {
private:
  Environment *env;
  Interpreter& interp;

  void put(const std::string& str, const std::function<Obj(const ArgList&, Interpreter&)> func);

public:
  PrimitivePutter(Environment *env, Interpreter& interp): env {env}, interp {interp} {}
  void put_numeric_functions();
  void put_data_functions();
  void put_predicates();
  void put_misc_functions();
  void put_all_functions();

};

}
