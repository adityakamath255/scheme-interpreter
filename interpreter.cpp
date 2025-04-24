#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "evaluation.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <unordered_map>
#include <string>
#include <string_view>

namespace Scheme {

Environment
Interpreter::install_global_environment() {
  auto env = new Environment();
  for (const auto& p : get_primitive_functions()) {
    env->define(make_symbol(p.first), new Primitive(p.second));
  }
  for (const auto& p : get_consts()) {
    env->define(make_symbol(p.first), p.second);
  }
  return Environment(env);
}

Interpreter::Interpreter(): 
  global_env {install_global_environment()}, 
  intern_table {} 
{}

Symbol
Interpreter::make_symbol(const std::string_view str) {
  auto itr = intern_table.find(str);
  if (itr == intern_table.end()) {
    auto new_str = new std::string(str);
    intern_table.insert({std::string_view(*new_str), new_str});
    return Symbol(new_str);
  }
  else {
    return Symbol(itr->second);
  }
}

Obj
Interpreter::interpret(const std::string& code) {
  const auto tokens = Lexer(code).tokenize();
  const auto AST_0 = Parser(tokens, *this).parse();
  auto AST_1 = classify(AST_0); 
  return as_obj(eval(AST_1, &global_env));
}

Interpreter::~Interpreter() {
  for (auto& [key, value] : intern_table) {
    delete value;
  }
}

}
