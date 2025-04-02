#include "common.hpp"
#include "expressions.cpp"

namespace scheme {

void
if_expr::tco() {
  consequent->tco();
  alternative->tco();
}

void
begin_expr::tco() {
  if (!actions.empty()) {
    actions.back()->tco();
  }
}

void
cond_expr::tco() {
  if_form->tco();
}

void
application::tco() {
  at_tail = true;
}

}
