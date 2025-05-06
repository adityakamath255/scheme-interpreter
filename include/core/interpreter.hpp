#pragma once
#include <core/types.hpp>
#include <core/environment.hpp>
#include <core/memory.hpp>
#include <unordered_map>
#include <string>
#include <string_view>
#include <chrono>

namespace Scheme {

class Interpreter {
private:
  std::unordered_map<std::string_view, std::string*> intern_table;
  Environment *global_env; 
  bool profiling;

  std::chrono::microseconds lexing_time {0};
  std::chrono::microseconds parsing_time {0};
  std::chrono::microseconds ast_building_time {0};
  std::chrono::microseconds evaluating_time {0};
  std::chrono::microseconds garbage_collecting_time {0};

  void install_global_environment();

public:
  Allocator alloc;

  Interpreter(bool);
  ~Interpreter();

  bool is_profiled() {return profiling;}
  Environment *get_global_env() {return global_env;}
  Symbol intern_symbol(const std::string_view);
  Obj interpret(const std::string&);
  void print_timings() const;

  template<typename T, typename... Args>
  T* spawn(Args&&... args) {
    return alloc.spawn<T>(std::forward<Args>(args)...);
  }
};

}
