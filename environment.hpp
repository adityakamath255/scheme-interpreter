#pragma once
#include "types.hpp"

namespace Scheme {

class Environment {
private:
  std::map<Symbol, Obj> frame {};
  Environment *const super;
  decltype(frame)::iterator assoc(const Symbol& s);
public:
  Environment();
  Environment(Environment *super_);
  void set_variable(const Symbol& s, const Obj obj);
  Obj lookup(const Symbol& s);
  void define_variable(const Symbol& s, Obj obj);
  Environment *extend(const vector<Symbol>& parameters, const vector<Obj>& arguments);
};

}
