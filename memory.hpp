#pragma once
#include "types.hpp"
#include <string>
#include <unordered_set>

namespace Scheme {

using LambdaBody = std::shared_ptr<Expression>;

class HeapEntity {
private:
  bool marked;

public:
  HeapEntity(): marked {false} {}

  void mark() {marked = true; }
  void unmark() {marked = false; }
  bool is_marked() {return marked; }
};

class Allocator {
private:
  std::vector<void*> live_memory;
  std::unordered_set<void*> marked;
public:
  Allocator(): live_memory {} {};
  Cons* make_cons(Obj car, Obj cdr); 
  Symbol make_symbol(const std::string& str); 
  Procedure* make_procedure(ParamList p, LambdaBody b, Environment* e);
  Environment* make_environment(); 
  Environment* make_environment(Environment *super);
  void mark_obj(Obj& obj); 
  void mark_cons(Cons& cons); 
  void mark_procedure(Procedure& proc); 
  void mark_environment(Environment& env);
  void sweep(); 
  void clean_up(Environment&);
};

}

