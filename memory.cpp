#include "types.hpp"
#include "environment.hpp"
#include "memory.hpp"

#include <iostream>

namespace Scheme {

void
Cons::mark_impl(HeapEntitySet& marked_set) {
  if (auto car_ent = try_get_heap_entity(car)) {
    car_ent->mark_recursive(marked_set);
  }
  if (auto cdr_ent = try_get_heap_entity(cdr)) {
    cdr_ent->mark_recursive(marked_set);
  }
}

void 
Primitive::mark_impl(HeapEntitySet& marked_set) {}

void 
Procedure::mark_impl(HeapEntitySet& marked_set) {
  env->mark_recursive(marked_set);
}

void
Environment::mark_impl(HeapEntitySet& marked_set) {
  for (auto& [key, value] : frame) {
    if (auto ent = try_get_heap_entity(value)) {
      ent->mark_recursive(marked_set);
    }
  }
  if (super) {
    super->mark_recursive(marked_set);
  }
}

Cons*
Allocator::make_cons(Obj car, Obj cdr) {
  auto ret = new Cons(std::move(car), std::move(cdr));
  live_memory.push_back(ret);
  return ret;
}

Primitive*
Allocator::make_primitive(Obj(*func)(const ArgList&, Interpreter&)) {
  auto ret = new Primitive(func);
  live_memory.push_back(ret);
  return ret;
}

Procedure*
Allocator::make_procedure(ParamList p, Expression *b, Environment* e) {
  auto ret = new Procedure(std::move(p), b, e);
  live_memory.push_back(ret);
  return ret;
}

Environment*
Allocator::make_environment() {
  auto ret = new Environment;
  live_memory.push_back(ret);
  return ret;
}

Environment*
Allocator::make_environment(Environment *super) {
  auto ret = new Environment(super);
  live_memory.push_back(ret);
  return ret;
}

void 
Allocator::mark(const HeapEntityVec& roots) {
  for (auto& root : roots) {
    root->mark_recursive(marked_set);
  }
}

void 
Allocator::unmark() {
  for (auto ptr : marked_set) {
    ptr->unmark();
  }
  marked_set.clear();
}

void
Allocator::sweep() {
  std::vector<HeapEntity*> new_memory;
  for (HeapEntity *ptr : live_memory) {
    if (marked_set.erase(ptr)) {
      ptr->unmark();
      new_memory.push_back(ptr);
    }
    else {
      delete ptr;
    }
  }
  live_memory = std::move(new_memory);
}

void
Allocator::cleanup() {
  sweep();
}

void
Allocator::cleanup(const HeapEntityVec& roots) {
  mark(roots);
  sweep();
  unmark();
}

}
