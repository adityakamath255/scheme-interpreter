#include "common.hpp"
#include "expressions.cpp"

namespace Scheme {

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
