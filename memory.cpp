#include "types.hpp"
#include "environment.hpp"
#include "memory.hpp"

namespace Scheme {

void
Cons::push_children(std::stack<HeapEntity*>& worklist) {
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
Primitive::push_children(std::stack<HeapEntity*>& worklist) {}

void 
Procedure::push_children(std::stack<HeapEntity*>& worklist) {
  if (env && !env->marked) {
    worklist.push(env);
  }
}

void
Environment::push_children(std::stack<HeapEntity*>& worklist) {
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
Allocator::mark(const std::vector<HeapEntity*>& roots) {
  std::stack<HeapEntity*> worklist;
  for (auto root : roots) {
    if (root && !root->marked) {
      worklist.push(root);
    }
  }

  while (!worklist.empty()) {
    auto curr = worklist.top();
    worklist.pop();

    if (!curr->marked) {
      curr->marked = true;
      curr->push_children(worklist);
    }
  }
}

void
Allocator::sweep() {
  auto itr = live_memory.begin();
  while (itr != live_memory.end()) {
    auto ptr = *itr;
    if (ptr->marked) {
      ptr->marked = false;
      itr++;
    }
    else {
      delete ptr;
      itr = live_memory.erase(itr);
    }
  }
}

void
Allocator::cleanup() {
  sweep();
}

void
Allocator::cleanup(const std::vector<HeapEntity*>& roots) {
  mark(roots);
  sweep();
}

}
