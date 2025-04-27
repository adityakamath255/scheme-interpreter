#include "types.hpp"
#include "environment.hpp"
#include "memory.hpp"

#include <iostream>

namespace Scheme {

void
Cons::push_children(HeapEntityStack& worklist) {
  if (auto car_ent = try_get_heap_entity(car)) {
    if (!car_ent->marked) {
      worklist.push(car_ent);
    }
  }
  if (auto cdr_ent = try_get_heap_entity(cdr)) {
    if (!cdr_ent->marked) {
      worklist.push(cdr_ent);
    }
  }
}

void 
Primitive::push_children(HeapEntityStack& worklist) {}

void 
Procedure::push_children(HeapEntityStack& worklist) {
  if (env && !env->marked) {
    worklist.push(env);
  }
}

void
Environment::push_children(HeapEntityStack& worklist) {
  for (auto& [key, value] : frame) {
    if (auto ent = try_get_heap_entity(value)) {
      if (!ent->marked) {
        worklist.push(ent);
      }
    }
  }
  if (super && !super->marked) {
    worklist.push(super);
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
  HeapEntityStack worklist;
  for (auto& root : roots) {
    if (root && !root->marked) {
      worklist.push(root);
    }
  }

  while (!worklist.empty()) {
    auto curr = worklist.top();
    worklist.pop();

    if (!curr->marked) {
      curr->marked = true;
      marked_set.insert(curr);
      curr->push_children(worklist);
    }
  }
}

void 
Allocator::unmark() {
  for (auto ptr : marked_set) {
    ptr->marked = false;
  }
  marked_set.clear();
}

void
Allocator::sweep() {
  std::vector<HeapEntity*> new_memory;
  for (HeapEntity *ptr : live_memory) {
    if (marked_set.erase(ptr)) {
      ptr->marked = false;
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
