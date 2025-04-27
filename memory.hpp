#pragma once
#include "types.hpp"
#include <string>
#include <unordered_set>

namespace Scheme {

using HeapEntityVec = std::vector<HeapEntity*>;
using HeapEntityStack = std::stack<HeapEntity*>;
using HeapEntitySet = std::unordered_set<HeapEntity*>;

inline HeapEntity*
try_get_heap_entity(const Obj& obj) {
  if (is_pair(obj)) {
    return as_pair(obj);
  }
  else if (is_primitive(obj)) {
    return as_primitive(obj);
  }
  else if (is_procedure(obj)) {
    return as_procedure(obj);
  }
  else {
    return nullptr;
  }
}

class Allocator {
private:
  HeapEntityVec live_memory;
  HeapEntitySet marked_set;
  void mark(const HeapEntityVec&);
  void unmark();
  void sweep(); 
public:
  Allocator(): live_memory {} {};
  void register_entity(HeapEntity *ent) {live_memory.push_back(ent);}
  Cons* make_cons(Obj car, Obj cdr); 
  Procedure* make_procedure(ParamList p, Expression *b, Environment* e);
  Primitive* make_primitive(Obj(*func)(const ArgList&, Interpreter&));
  Environment* make_environment(); 
  Environment* make_environment(Environment *super);
  void cleanup();
  void cleanup(const HeapEntityVec&);
};

}

