#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "evaluation.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "memory.hpp"
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
    total {total},
    start {std::chrono::high_resolution_clock::now()}
  {}

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    total += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  }
};

Environment
Interpreter::install_global_environment() {
  Environment env;
  for (const auto& p : get_primitive_functions()) {
    env.define(intern_symbol(p.first), new Primitive(p.second));
  }
  for (const auto& p : get_consts()) {
    env.define(intern_symbol(p.first), p.second);
  }
  return env;
}

Interpreter::Interpreter(bool profiling): 
  global_env {install_global_environment()}, 
  intern_table {},
  profiling {profiling}
{}

Interpreter::~Interpreter() {
  for (auto& [key, value] : intern_table) {
    delete value;
  }
}

Symbol
Interpreter::intern_symbol(const std::string_view str) {
  auto itr = intern_table.find(str);
  if (itr == intern_table.end()) {
    auto new_str = new std::string(str);
    intern_table.emplace(*new_str, new_str);
    return Symbol(new_str);
  }
  else {
    return Symbol(itr->second);
  }
}

Obj
Interpreter::interpret(const std::string& code) {
  if (profiling) {
    auto tokens = [&](){
      Timer timer(lexing_time);
      return Lexer(code).tokenize();
    }();

    auto s_expr = [&](){
      Timer timer(parsing_time);
      return Parser(tokens, *this).parse();
    }();

    auto ast = [&](){ 
      Timer timer(classifying_time);
      return classify(s_expr);
    }();

    auto result = [&](){
      Timer timer(evaluating_time);
      return ast->eval(&global_env, *this);
    }();

    return as_obj(result);
  } 
  else {
    auto tokens = Lexer(code).tokenize();
    auto s_expr = Parser(tokens, *this).parse();
    auto ast = classify(s_expr); 
    auto result = ast->eval(&global_env, *this);
    return as_obj(result);
  }
}

void
Interpreter::print_timings() const {
  using namespace std::chrono;
  std::cout << "Profile:\n"
    << "Lexing:      " << duration_cast<microseconds>(lexing_time).count()      << " μs\n"
    << "Parsing:     " << duration_cast<microseconds>(parsing_time).count()     << " μs\n"
    << "Classifying: " << duration_cast<microseconds>(classifying_time).count() << " μs\n"
    << "Evaluating:  " << duration_cast<microseconds>(evaluating_time).count()  << " μs\n";
}

}
