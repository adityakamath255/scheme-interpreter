#include "types.hpp"
#include "environment.hpp"
#include "expressions.hpp"
#include "memory.hpp"

namespace Scheme {

void String::push_children(MarkStack&) {}

void
Cons::push_children(MarkStack& worklist) {
  if (auto car_ent = try_get_heap_entity(car)) {
    worklist.push(car_ent);
  }
  if (auto cdr_ent = try_get_heap_entity(cdr)) {
    worklist.push(cdr_ent);
  }
}

void 
Vector::push_children(MarkStack& worklist) {
  for (Obj& obj : data) {
    if (auto ent = try_get_heap_entity(obj)) {
      worklist.push(ent);
    }
  }
}

void Primitive::push_children(MarkStack&) {}

void 
Procedure::push_children(MarkStack& worklist) {
  worklist.push(body);
  worklist.push(env);
}

void
Environment::push_children(MarkStack& worklist) {
  for (auto& [key, value] : frame) {
    if (auto ent = try_get_heap_entity(value)) {
      worklist.push(ent);
    }
  }
  if (super) {
    worklist.push(super);
  }
}

void Literal::push_children(MarkStack& worklist) {
  if (auto ent = try_get_heap_entity(obj)) {
    worklist.push(ent);
  }
}

void Variable::push_children(MarkStack& worklist) {}

void 
Quoted::push_children(MarkStack& worklist) {
  if (auto ent = try_get_heap_entity(text)) {
    worklist.push(ent);
  }
}

void
Quasiquoted::push_children(MarkStack& worklist) {
  if (std::holds_alternative<Obj>(text)) {
    if (auto ent = try_get_heap_entity(get<Obj>(text))) {
      worklist.push(ent);
    }
  }
  else {
    for (auto expr : get<std::vector<Expression*>>(text)) {
      worklist.push(expr);
    }
  }
}

void
Set::push_children(MarkStack& worklist) {
  worklist.push(value);
}

void
If::push_children(MarkStack& worklist) {
  worklist.push(predicate);
  worklist.push(consequent);
  worklist.push(alternative);
}

void 
Begin::push_children(MarkStack& worklist) {
  for (auto& action : actions) {
    worklist.push(action);
  }
}

void
Lambda::push_children(MarkStack& worklist) {
  worklist.push(body);
}

void
Define::push_children(MarkStack& worklist) {
  worklist.push(value);
}

void 
Let::push_children(MarkStack& worklist) {
  for (auto& [key, value] : bindings) {
    worklist.push(value);
  }
  worklist.push(body);
}

void 
LetSeq::push_children(MarkStack& worklist) {
  for (auto& [key, value] : bindings) {
    worklist.push(value);
  }
  worklist.push(body);
}

void
Cond::push_children(MarkStack& worklist) {
  for (auto& clause : clauses) {
    if (clause.predicate) {
      worklist.push(clause.predicate);
    }
    if (clause.actions) {
      worklist.push(clause.actions);
    }
  }
}

void 
Application::push_children(MarkStack& worklist) {
  worklist.push(op);
  for (auto& param : params) {
    worklist.push(param);
  }
}

void
And::push_children(MarkStack& worklist) {
  for (auto& expr : exprs) {
    worklist.push(expr);
  }
}

void
Or::push_children(MarkStack& worklist) {
  for (auto& expr : exprs) {
    worklist.push(expr);
  }
}

void
Cxr::push_children(MarkStack& worklist) {
  worklist.push(expr);
}

void 
Allocator::mark(const std::vector<HeapEntity*>& roots) {
  MarkStack worklist;
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
  auto is_dead = [](HeapEntity *ptr) {
    if (ptr->marked) {
      ptr->marked = false;
      return false;
    }
    else {
      delete ptr;
      return true;
    }
  };

  std::erase_if(live_memory, is_dead);
}

void
Allocator::recycle() {
  sweep();
}

void
Allocator::recycle(const std::vector<HeapEntity*>& roots) {
  mark(roots);
  sweep();
}

}
