#include "common.hpp"

namespace Scheme {

static void display(const bool b) {
  cout << (b ? "#t" : "#f");
}

static void display(const double n) {
  cout << n;
}

static void display(const Symbol& s) {
  cout << s.name;
}

static void display(const string& w) {
  cout << w;
}

static void display(const nullptr_t x) {}

static void display(const Procedure *p) {
  cout << "Procedure at " << p;
}

static void display(const Primitive *p) {
  cout << "Primitive at " << p;
}

static void display(Cons *const c);

static struct {
  template<typename T>
  void operator()(T x) {
    display(x);
  }
} display_overload;

static Obj display_iter(const Obj obj, const bool head) {
  if (is_pair(obj)) {
    if (!head)
      cout << " ";
    const auto c = get<Cons*>(obj);
    visit(display_overload, c->car);
    return display_iter(c->cdr, 0);
  } 
  else {
    return obj;
  }
}

static void display(Cons *const c) {
  cout << "(";
  Obj obj = display_iter(c, true);
  if (!is_null(obj)) {
    cout << " . ";
    visit(display_overload, obj);
  }
  cout << ")";
}

void display(const Obj obj) {
  visit(display_overload, obj);
}

}

