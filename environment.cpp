#include "types.hpp"
#include "environment.hpp"
#include "interpreter.hpp"
#include <unordered_map>

namespace Scheme {

std::pair<Obj&, int>
Environment::get_impl(const Symbol& s, const int depth) {
  const auto found = frame.find(s);
  if (found != frame.end()) {
    return {found->second, depth};
  }
  else if (super != nullptr) {
    return super->get_impl(s, depth + 1);
  }
  else {
    throw std::runtime_error("unbound variable: " + s.get_name());
  }
}

Obj&
Environment::get(const Symbol& s) {
  return get_impl(s, 0).first;
}

std::pair<Obj&, int>
Environment::get_with_depth(const Symbol& s) {
  return get_impl(s, 0);
}

void
Environment::set(const Symbol& s, const Obj obj) {
  get(s) = std::move(obj);
}

void
Environment::define(const Symbol& s, Obj obj) {
  frame[s] = std::move(obj);
}

Environment*
Environment::extend(Interpreter& interp) {
  return interp.spawn<Environment>(this);
}

Environment*
Environment::extend(const ParamList& parameters, const ArgList& arguments, Interpreter& interp) {
  if (parameters.size() != arguments.size()) {
    throw std::runtime_error("env extend size mismatch");
  }
  auto ret = extend(interp);
  for (size_t i = 0; i < parameters.size(); i++) {
    ret->define(parameters[i], arguments[i]);
  }
  return ret;
}

}
