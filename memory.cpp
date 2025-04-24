#include "types.hpp"

namespace Scheme {

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
  std::unordered_map<void*> marked;

public:
  Allocator(): live_memory {} {}

  Cons*
  make_cons(Obj car, Obj cdr) {
    auto ret = new Cons(std::move(car), std::move(cdr));
    live_memory.push_back(ret);
    return ret;
  }

  Symbol
  make_symbol(const std::string& str) {
    return Symbol(str, &live_memory);
  }

  Procedure*
  make_procedure(ParamList p, Expression* b, Environment* e) {
    auto ret = new Procedure(std::move(p), b, e);
    live_memory.push_back(ret);
    return ret;
  }

  Environment*
  make_environment() {
    auto ret = new Environment();
    live_memory.push_back(ret);
    return ret;
  }

  Environment*
  make_environment(Environment *super) {
    auto ret = super->extend();
    live_memory.push_back(ret);
    return ret;
  }

  void
  mark_obj(Obj& obj) {
    if (is_pair(obj)) {
      mark_pair(as_pair(obj));
    else if (is_procedure(obj)) {
      mark_procedure(as_procedure(obj));
    }
  }

  void
  mark_cons(Cons& cons) {
    if (!cons.is_marked()) {
      cons.mark();    
      marked.insert(cons);
      mark_obj(cons.car);
      mark_obj(cons.cdr);
    }
  }
  
  void 
  mark_procedure(Procedure& proc) {
    if (!proc.is_marked()) {
      proc.mark();
      marked.insert(proc);
      mark_environment(proc.env);
    }
  }

  void 
  mark_environment(Environment& env) {
    if (!env.is_marked()) {
      env.mark();
      marked.insert(env);
      for (auto& [key, value] : env->frame) {
        mark_obj(value);
      }
    }
  }

  void
  mark_all() {
    marked.clear();
    mark_environment(global);
  }

  void
  sweep_all() {
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
  clean_up() {
    mark_all();
    sweep_all();
  }
};

}

