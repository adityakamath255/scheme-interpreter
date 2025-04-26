#pragma once
#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "primitives.hpp"
#include "memory.hpp"
#include <unordered_map>
#include <string>
#include <string_view>

namespace Scheme {

class Interpreter {
private:
  std::unordered_map<std::string_view, std::string*> intern_table;
  Environment global_env;

  Environment install_global_environment();

public:
  Allocator alloc;

  Interpreter();
  ~Interpreter();

  Symbol make_symbol(const std::string_view);
  Obj interpret(const std::string&);

};

}
