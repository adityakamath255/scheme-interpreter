#pragma once
#include "types.hpp"
#include <unordered_map>

namespace Scheme {

class Allocator;

class Environment : public HeapEntity {
  friend Allocator;

private:
  std::unordered_map<Symbol, Obj> frame {};
  std::pair<Obj&, int> get_impl(const Symbol&, const int);
  void mark_impl(HeapEntitySet&) override;

public:
  Environment *const super;
  Environment(): frame {}, super {nullptr} {};
  Environment(Environment *super): frame {}, super {super} {};
  Obj& get(const Symbol&);
  std::pair<Obj&, int> get_with_depth(const Symbol&);
  void set(const Symbol&, const Obj);
  void define(const Symbol&, Obj);
  Environment *extend(Interpreter&);
  Environment *extend(const ParamList&, const ArgList&, Interpreter&);
};

}
