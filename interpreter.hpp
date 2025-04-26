#pragma once
#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "primitives.hpp"
#include "memory.hpp"
#include <unordered_map>
#include <string>
#include <string_view>
#include <chrono>

namespace Scheme {

class Interpreter {
private:
  std::unordered_map<std::string_view, std::string*> intern_table;
  Environment global_env;
  bool profiling;

  std::chrono::microseconds lexing_time {0};
  std::chrono::microseconds parsing_time {0};
  std::chrono::microseconds classifying_time {0};
  std::chrono::microseconds evaluating_time {0};

  Environment install_global_environment();

public:
  Allocator alloc;

  Interpreter(bool);
  ~Interpreter();

  bool is_profiled() {return profiling;}
  Symbol intern_symbol(const std::string_view);
  Obj interpret(const std::string&);
  void print_timings() const;

};

}
