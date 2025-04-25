#include "types.hpp"
#include "environment.hpp"
#include "memory.hpp"

namespace Scheme {

Cons*
Allocator::make_cons(Obj car, Obj cdr) {
  auto ret = new Cons(std::move(car), std::move(cdr));
  //live_memory.push_back(ret);
  return ret;
}

Procedure*
Allocator::make_procedure(ParamList p, Expression *b, Environment* e) {
  auto ret = new Procedure(std::move(p), b, e);
  //live_memory.push_back(ret);
  return ret;
}

Environment*
Allocator::make_environment() {
  auto ret = new Environment;
  //live_memory.push_back(ret);
  return ret;
}

Environment*
Allocator::make_environment(Environment *super) {
  auto ret = new Environment(super);
  //live_memory.push_back(ret);
  return ret;
}

/*
void
Allocator::mark_obj(Obj& obj) {
  if (is_pair(obj)) {
    mark_pair(as_pair(obj));
  else if (is_procedure(obj)) {
    mark_procedure(as_procedure(obj));
  }
}

void
Allocator::mark_cons(Cons& cons) {
  if (!cons.is_marked()) {
    cons.mark();    
    marked.insert(cons);
    mark_obj(cons.car);
    mark_obj(cons.cdr);
  }
}

void 
Allocator::mark_procedure(Procedure& proc) {
  if (!proc.is_marked()) {
    proc.mark();
    marked.insert(proc);
    mark_environment(proc.env);
  }
}

void 
Allocator::mark_environment(Environment& env) {
  if (!env.is_marked()) {
    env.mark();
    marked.insert(env);
    for (auto& [key, value] : env->frame) {
      mark_obj(value);
    }
  }
}

void
Allocator::sweep_all() {
  for (void *ptr : live_memory) {
    auto itr = marked.find(ptr);
    if (itr == marked.end()) {
      delete itr;
    }
    else {
      marked.erase(itr);
    }
  }
}

void
Allocator::clean_up(Environment& global) {
  marked.clear();
  mark_environment(global);
  sweep();
}

*/

}
