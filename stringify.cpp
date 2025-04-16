#include "common.hpp"
#include "primitives.hpp"
#include <sstream>
#include <format>

namespace Scheme {

static string 
stringify(const bool b) {
  return (b ? "#t" : "#f");
}

static string 
stringify(const double n) {
  std::ostringstream ret;
  ret << n;
  return ret.str();
}

static string 
stringify(const Symbol& s) {
  return s.name;
}

static string 
stringify(const string& w) {
  return w;
}

static string 
stringify(const nullptr_t x) {
  return "()";
}

static string 
stringify(const Void x) {
  return "#!void";
}

static string 
stringify(const Procedure *p) {
  std::ostringstream ret;
  ret << "procedure at " << static_cast<const void*>(p);
  return ret.str();
}

static string 
stringify(const Primitive *p) {
  std::ostringstream ret;
  ret << "procedure at " << static_cast<const void*>(p);
  return ret.str();
}

static string
stringify(Cons *const ls) {
  std::ostringstream ret;
  ret << "(" + stringify(ls->car);
  Obj obj;
  for (obj = ls->cdr; is_pair(obj); obj = get<Cons*>(obj)->cdr) {
    ret << " ";
    ret << stringify(get<Cons*>(obj)->car);
  }
  if (!is_null(obj)) {
    ret << " . ";
    ret << stringify(obj);
  }
  ret << ")";
  return ret.str();
}

static struct {
  template<typename T>
  string operator
  ()(T x) {
    return stringify(x);
  }
} stringify_overload;

string 
stringify(const Obj obj) {
  return visit(stringify_overload, obj);
}

}

