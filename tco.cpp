#include "types.hpp"
#include "expressions.hpp"
#include "tco.hpp"

namespace Scheme {

TailCall::TailCall(Obj proc, vector<Obj> args): 
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
  if_form->tco();
}

void
Application::tco() {
  at_tail = true;
}

}
