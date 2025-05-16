#pragma once
#include <interpreter/types.hpp>

namespace Scheme {

class Interpreter;

class BuiltinInstaller {
private:
  Environment *env;
  Interpreter& interp;

  void install(const std::string& str, const std::function<Obj(const ArgList&, Interpreter&)> func);

public:
  BuiltinInstaller(Environment *env, Interpreter& interp): env {env}, interp {interp} {}
  void install_numeric_functions();
  void install_data_functions();
  void install_predicates();
  void install_misc_functions();
  void install_all_functions();

};

}
