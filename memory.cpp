#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "memory.hpp"

namespace Scheme {

void
Cons::push_children(std::stack<HeapEntity*>& worklist) {
  if (auto car_ent = try_get_heap_entity(car)) {
    worklist.push(car_ent);
  }
  if (auto cdr_ent = try_get_heap_entity(cdr)) {
    worklist.push(cdr_ent);
  }
}

void 
Primitive::push_children(std::stack<HeapEntity*>& worklist) {}

void 
Procedure::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(body);
  worklist.push(env);
}

void
Environment::push_children(std::stack<HeapEntity*>& worklist) {
  for (auto& [key, value] : frame) {
    if (auto ent = try_get_heap_entity(value)) {
      worklist.push(ent);
    }
  }
  if (super) {
    worklist.push(super);
  }
}

void
Literal::push_children(std::stack<HeapEntity*>& worklist) {}

void  
Variable::push_children(std::stack<HeapEntity*>& worklist) {}

void 
Quoted::push_children(std::stack<HeapEntity*>& worklist) {
  if (auto ent = try_get_heap_entity(text_of_quotation)) {
    worklist.push(ent);
  }
}

void
Set::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(value);
}

void
If::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(predicate);
  worklist.push(consequent);
  worklist.push(alternative);
}

void 
Begin::push_children(std::stack<HeapEntity*>& worklist) {
  for (auto& action : actions) {
    worklist.push(action);
  }
}

void
Lambda::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(body);
}

void
Define::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(value);
}

void 
Let::push_children(std::stack<HeapEntity*>& worklist) {
  for (auto& [key, value] : bindings) {
    worklist.push(value);
  }
  worklist.push(body);
}

void
Cond::push_children(std::stack<HeapEntity*>& worklist) {
  for (auto& clause : clauses) {
    worklist.push(clause.predicate);
    worklist.push(clause.actions);
  }
}

void 
Application::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(op);
  for (auto& param : params) {
    worklist.push(param);
  }
}

void
And::push_children(std::stack<HeapEntity*>& worklist) {
  for (auto& expr : exprs) {
    worklist.push(expr);
  }
}

void
Or::push_children(std::stack<HeapEntity*>& worklist) {
  for (auto& expr : exprs) {
    worklist.push(expr);
  }
}

void
Cxr::push_children(std::stack<HeapEntity*>& worklist) {
  worklist.push(expr);
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
