#include "types.hpp"
#include "environment.hpp"
#include <unordered_map>

namespace Scheme {

std::unordered_map<Symbol, Obj>::iterator
Environment::assoc(const Symbol& s) {
  const auto found = frame.find(s);
  if (found != frame.end()) {
    return found;
  }
  else if (super != nullptr) {
    return super->assoc(s);
  }
  else {
    throw std::runtime_error("unbound variable: " + s.get_name());
  }
  return found;
}

Environment::Environment():
  super {nullptr} {}
  
Environment::Environment(Environment *super_): 
  super {super_} {}

void
Environment::set_variable(const Symbol& s, const Obj obj) {
  assoc(s)->second = std::move(obj);
}

Obj 
Environment::lookup(const Symbol& s) {
  return assoc(s)->second;
}

void
Environment::define_variable(const Symbol& s, Obj obj) {
  const auto found = frame.find(s);
  if (found != frame.end()) {
    throw std::runtime_error("binding already present: " + s.get_name());
  }
  else {
    frame.insert({s, std::move(obj)});
  }
}

Environment*
Environment::extend() {
  return new Environment(this);
}

Environment*
Environment::extend(const ParamList& parameters, const ArgList& arguments) {
  if (parameters.size() != arguments.size()) {
    throw std::runtime_error("env extend size mismatch");
  }
  Environment *ret = new Environment(this);
  for (int i = 0; i < parameters.size(); i++) {
    ret->define_variable(parameters[i], arguments[i]);
  }
  return ret;
}

}
