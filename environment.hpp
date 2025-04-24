#pragma once
#include "types.hpp"
#include <unordered_map>

namespace Scheme {

class Environment {
private:
  std::unordered_map<Symbol, Obj> frame {};
  Environment *const super;
  decltype(frame)::iterator assoc(const Symbol&);
public:
  Environment();
  Environment(Environment*);
  void set_variable(const Symbol&, const Obj);
  Obj lookup(const Symbol&);
  void define_variable(const Symbol&, Obj);
  Environment *extend();
  Environment *extend(const ParamList&, const ArgList&);
};

}
