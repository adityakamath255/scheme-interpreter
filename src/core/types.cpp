#include <core/types.hpp>
#include <string>
#include <sstream>
#include <format>

namespace Scheme {

Obj 
Cons::at(const std::string& s) {
  Obj curr = const_cast<Cons*>(this);

  if (*s.begin() != 'c' || *s.rbegin() != 'r') {
    throw std::runtime_error("invalid cons operation: " + s);
  }

  for (size_t i = s.size() - 2; i > 0; i--) {
    switch (s[i]) {
      case 'a':
        curr = as_pair(curr)->car;
        break;
      case 'd':
        curr = as_pair(curr)->cdr;
        break;
      default:
        throw std::runtime_error("invalid cons operation: " + s);
    }
  }

  return curr;
}

std::pair<int, bool>
list_profile(const Obj ls) {
  if (is_null(ls)) {
    return {0, true};
  }
  else {
    int len = 1;
    Obj obj = as_pair(ls)->cdr;
    while (is_pair(obj)) {
      len++;
      obj = as_pair(obj)->cdr;
    } 
    if (is_null(obj)) {
      return {len, true};
    }
    else {
      return {len + 1, false};
    }
  }
}

bool
equal(const Obj obj_0, const Obj obj_1) {
  if (!same_type(obj_0, obj_1)) {
    return false;
  }
  else {
    return std::visit(Overloaded{
      [=](bool) -> bool {
        return obj_0 == obj_1;
      },

      [=](double) -> bool {
        return obj_0 == obj_1;
      },

      [=](char) -> bool {
        return obj_0 == obj_1;
      },

      [=](Symbol) -> bool {
        return obj_0 == obj_1;
      },

      [=](Procedure*) -> bool {
        return obj_0 == obj_1;
      },

      [=](Builtin*) -> bool {
        return obj_0 == obj_1;
      },

      [=](Null) -> bool {
        return true;
      },

      [=](Void) -> bool {
        return true;
      },

      [=](String*) -> bool {
        return as_string(obj_0)->data == as_string(obj_1)->data;
      },
      
      [=](Cons*) -> bool {
        return (
          equal(as_pair(obj_0)->car, as_pair(obj_1)->car) &&
          equal(as_pair(obj_0)->cdr, as_pair(obj_1)->cdr)
        );
      },

      [=](Vector*) -> bool {
        if (as_vector(obj_0)->data.size() != as_vector(obj_1)->data.size()) {
          return false;
        }
        else {
          auto itr_0 = as_vector(obj_0)->data.begin();
          auto itr_1 = as_vector(obj_1)->data.begin();
          while (itr_0 != as_vector(obj_0)->data.end()) {
            if (!equal(*itr_0, *itr_1)) {
              return false;
            }
            itr_0++;
            itr_1++;
          }
          return true;
        }
      },

    }, obj_0);
  }
}
std::string 
stringify(const Obj obj) {
  return std::visit(Overloaded{
    [](const bool b) -> std::string {
      return b ? "#t" : "#f";
    },

    [](const double n) -> std::string {
      return std::format("{}", n);
    },

    [](const char n) -> std::string {
      return std::format("#\\{}", n);
    },

    [](const Symbol& s) -> std::string {
      return s.get_name();
    },

[](const String *w) -> std::string {
      return w->data;
    },
    
    [](Cons* const ls) -> std::string {
      if (!ls) {
        return "()";
      }
      
      std::ostringstream ret;
      ret << "(" << stringify(ls->car);
      
      Obj curr = ls->cdr;
      while (is_pair(curr)) {
        ret << " " << stringify(as_pair(curr)->car);
        curr = as_pair(curr)->cdr;
      }
      
      if (!is_null(curr)) {
        ret << " . " << stringify(curr);
      }
      
      ret << ")";
      return ret.str();
    },

    [](Vector* const v) -> std::string {
      if (v->data.empty()) {
        return "#()";
      }
      std::ostringstream ret;
      ret << "#(" << stringify(v->data[0]);
      for (size_t i = 1; i < v->data.size(); i++) {
        ret << " " << stringify(v->data[i]);
      }
      ret << ")";
      return ret.str();
    },

    [](const Procedure* p) -> std::string {
      return std::format("<procedure at {}>", static_cast<const void*>(p));
    },

    [](const Builtin* p) -> std::string {
      return std::format("<procedure at {}>", static_cast<const void*>(p));
    },

    [](const Null) -> std::string {
      return "()";
    },

    [](const Void) -> std::string {
      return "#<void>";
    },

  }, obj);
}

}

