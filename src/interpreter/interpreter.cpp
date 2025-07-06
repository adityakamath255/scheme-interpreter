#include <interpreter/types.hpp>
#include <interpreter/environment.hpp>
#include <interpreter/expressions.hpp>
#include <interpreter/interpreter.hpp>
#include <interpreter/memory.hpp>
#include <interpreter/lexer.hpp>
#include <interpreter/parser.hpp>
#include <builtins/installer.hpp>
#include <unordered_map>
#include <string>
#include <string_view>
#include <iostream>

namespace Scheme {

class Timer {
private:
  std::chrono::high_resolution_clock::time_point start;
  std::chrono::microseconds& total;

public:
  Timer(std::chrono::microseconds& total): 
    start {std::chrono::high_resolution_clock::now()},
    total {total}
  {}

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    total += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  }
};

void
Interpreter::install_global_environment() {
  global_env = alloc.spawn<Environment>();
  BuiltinInstaller(global_env, *this).install_all_functions();
}

Interpreter::Interpreter(bool profiling): 
  intern_table {},
  global_env {},
  profiling {profiling},
  alloc {}
{
  install_global_environment();
}

Interpreter::~Interpreter() {
  for (auto& [key, value] : intern_table) {
    delete value;
  }
  alloc.recycle();
}

Symbol
Interpreter::intern_symbol(const std::string_view str) {
  auto itr = intern_table.find(str);
  if (itr == intern_table.end()) {
    auto new_str = new std::string(str);
    intern_table.emplace(*new_str, new_str);
    return Symbol {new_str};
  }
  else {
    return Symbol {itr->second};
  }
}

Obj
Interpreter::interpret(const std::string& code) {
  if (profiling) {
    auto tokens = [&](){
      Timer timer(lexing_time);
      return Lexer(code).all_tokens();
    }();

    auto s_expr = [&](){
      Timer timer(parsing_time);
      return Parser(tokens, *this).parse();
    }();

    auto ast = [&](){ 
      Timer timer(ast_building_time);
      return build_ast(s_expr, *this);
    }();

    auto result = [&](){
      Timer timer(evaluating_time);
      return as_obj(ast->eval(global_env, *this));
    }();

    {
      Timer timer(garbage_collecting_time);
      std::vector<HeapEntity*> roots {global_env};
      if (auto ent = try_get_heap_entity(result)) {
        roots.push_back(ent);
      }
      alloc.recycle(roots);
    }
    
    return result;

  } 
  else {
    auto tokens = Lexer(code).all_tokens();
    auto s_expr = Parser(tokens, *this).parse();
    auto ast = build_ast(s_expr, *this); 
    auto result = as_obj(ast->eval(global_env, *this));
    std::vector<HeapEntity*> roots {global_env};
    if (auto ent = try_get_heap_entity(result)) {
      roots.push_back(ent);
    }
    alloc.recycle(roots);
    return result;
  }
}

void
Interpreter::print_timings() const {
  using namespace std::chrono;
  std::cout << "Profile:\n\n"
    << "Lexing:             " << duration_cast<microseconds>(lexing_time).count()             << " μs\n"
    << "Parsing:            " << duration_cast<microseconds>(parsing_time).count()            << " μs\n"
    << "AST Building:       " << duration_cast<microseconds>(ast_building_time).count()        << " μs\n"
    << "Evaluating:         " << duration_cast<microseconds>(evaluating_time).count()         << " μs\n"
    << "Garbage Collecting: " << duration_cast<microseconds>(garbage_collecting_time).count() << " μs\n";
  std::cout.flush();
}

}
