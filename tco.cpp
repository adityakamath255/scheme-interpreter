#include "types.hpp"
#include "expressions.hpp"
#include "tco.hpp"

namespace Scheme {

TailCall::TailCall(Obj proc, ArgList args): 
  proc {std::move(proc)}, 
  args {std::move(args)} 
{}

void
If::tco() {
  consequent->tco();
  alternative->tco();
}

void
Begin::tco() {
  if (!actions.empty()) {
    actions.back()->tco();
  }
}

void
Cond::tco() {
  for (auto& clause : clauses) {
    clause.actions->tco();
  }
}

void
Application::tco() {
  at_tail = true;
}

}
