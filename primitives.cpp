#include "common.hpp"
using namespace std;

namespace Scheme {

void 
display(const bool b) {
  cout << (b ? "#t" : "#f");
}

void 
display(const double n) {
  cout << n;
}

void 
display(const Symbol& s) {
  cout << s.name;
}

void 
display(const string& w) {
  cout << w;
}

void 
display(const nullptr_t x) {}

void 
display(const Procedure *p) {
  cout << "Procedure at " << p;
}

void 
display(const Primitive *p) {
  cout << "Primitive at " << p;
}

void 
display(Cons *const);

struct {
  template<typename T>
  void 
  operator()(T x) {
    display(x);
  }
} display_overload;

Obj
display_iter(const Obj obj, const bool head) {
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

void 
display(Cons *const c) {
  cout << "(";
  Obj obj = \
  display_iter(c, 1);

  if (!is_null(obj)) {
    cout << " . ";
    visit(display_overload, obj);
  }
  cout << ")";
}

void 
display(const Obj obj) {
  visit(display_overload, obj);
}

namespace PrimEnv {
  Obj
  display_final(const vector<Obj>& args) {
    for (const auto& arg : args) {
      display(arg);
    }
    return Symbol("ok");
  }

  Obj
  add(const vector<Obj>& args) {
    double ret = 0.0;
    for (const auto& arg : args) {
      ret += get<double>(arg);
    }
    return ret;
  }

  Obj
  sub(const vector<Obj>& args) {
    if (args.size() == 1)
      return -get<double>(args[0]);
    else
      return get<double>(args[0]) - get<double>(args[1]);   
  }

  Obj
  mul(const vector<Obj>& args) {
    double ret = 1.0;
    for (const auto& arg : args) {
      ret *= get<double>(arg);
    }
    return ret;
  }

  Obj
  div(const vector<Obj>& args) {
    return get<double>(args[0]) / get<double>(args[1]);
  }

  Obj 
  exp(const vector<Obj>& args) {
    return pow(get<double>(args[0]), get<double>(args[1]));
  }

  Obj 
  remainder(const vector<Obj>& args) {
    return fmod(get<double>(args[0]), get<double>(args[1]));
  }

  Obj
  lt(const vector<Obj>& args) {
    return get<double>(args[0]) < get<double>(args[1]);
  }

  Obj
  gt(const vector<Obj>& args) {
    return get<double>(args[0]) > get<double>(args[1]);
  }

  Obj 
  eq(const vector<Obj>& args) {
    return abs(get<double>(args[0]) - get<double>(args[1])) < precision;
  }

  Obj 
  le(const vector<Obj>& args) {
    return get<double>(args[0]) <= get<double>(args[1]);
  }

  Obj
  ge(const vector<Obj>& args) {
    return get<double>(args[0]) >= get<double>(args[1]);
  }

  Obj
  neq(const vector<Obj>& args) {
    return !get<bool>(eq(args));
  }

  Obj
  is_equal(const vector<Obj>& args) {
    return args[0] == args[1];
  }

  Obj
  cons_fn(const vector<Obj>& args) {
    return new Cons(args[0], args[1]);
  }

  Obj
  list_fn(const vector<Obj>& args) {
    Obj ret = nullptr;
    for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
      ret = new Cons(*curr, ret);
    }
    return ret;
  }

  Obj
  null_fn(const vector<Obj>& args) {
    return holds_alternative<nullptr_t>(args[0]);
  }

  Obj
  pair_fn(const vector<Obj>& args) {
    return is_pair(args[0]);
  }

  Obj
  id(const vector<Obj>& args) {
    return args[0];
  }

  Obj
  inc(const vector<Obj>& args) {
    return 1 + get<double>(args[0]);
  };

  Obj
  dec(const vector<Obj>& args) {
    return -1 + get<double>(args[0]);
  };

  Obj
  avg(const vector<Obj>& args) {
    return get<double>(add(args)) / args.size();
  };

  Obj
  sq(const vector<Obj>& args) {
    return get<double>(args[0]) * get<double>(args[0]);
  }

  Obj
  abs_fn(const vector<Obj>& args) {
    return abs(get<double>(args[0]));
  }

  Obj
  sin_fn(const vector<Obj>& args) {
    return sin(get<double>(args[0]));
  }

  Obj
  cos_fn(const vector<Obj>& args) {
    return cos(get<double>(args[0]));
  }

  Obj
  log_fn(const vector<Obj>& args) {
    return log(get<double>(args[0]));
  }

  Obj
  max_fn(const vector<Obj>& args) {
    double ret = -INFINITY;
    for (const auto& arg : args)
      ret = max(ret, get<double>(arg));
    return ret;
  }

  Obj
  min_fn(const vector<Obj>& args) {
    double ret = -INFINITY;
    for (const auto& arg : args)
      ret = min(ret, get<double>(arg));
    return ret;
  }

  Obj
  quotient(const vector<Obj>& args) {
    return static_cast<double>(
      static_cast<int>(
        get<double>(args[0]) / 
        get<double>(args[1])));
  }

  Obj
  sqrt_fn(const vector<Obj>& args) {
    return sqrt(get<double>(args[0]));
  }

  Obj
  not_fn(const vector<Obj>& args) {
    return !get<bool>(args[0]);
  }

  Obj
  new_line(const vector<Obj>& args) {
    cout << "\n";
    return true;
  }

  Obj
  list_len(const vector<Obj>& args) {
    double ret = 0;
    auto curr = args[0];
    while (is_pair(curr)) {
      ret++;
      curr = get<Cons*>(curr)->cdr;
    }
    return ret;
  }

  Obj
  append_rec(const Obj list1, const Obj list2) {
    if (is_pair(list1)) {
      return new Cons(
        get<Cons*>(list1)->car,
        append_rec(get<Cons*>(list1)->cdr, list2)
      );
    }
    else {
      return list2;
    }
  }

  Obj
  append(const vector<Obj>& args) {
    return append_rec(args[0], args[1]);
  }

  Obj
  error_fn(const vector<Obj>& args) {
    cout << "ERROR: ";
    for (auto obj : args) {
      display(obj);
      cout << " ";
    }
    return nullptr;
  }
}

using namespace PrimEnv;

const vector<pair<string, function<Obj(const vector<Obj>&)>>> 
prims = {
  {"+", add},
  {"-", sub},
  {"*", mul},
  {"/", div},
  {"**", exp},
  {"<", lt},
  {">", gt},
  {"=", eq},
  {"<=", le},
  {">=", ge},
  {"!=", neq},
  {"inc", inc},
  {"dec", dec},
  {"id", id},
  {"avg", avg},
  {"sq", sq},
  {"eq?", is_equal},
  {"equal?", is_equal},
  {"not", not_fn},
  {"cons", cons_fn},
  {"list", list_fn},
  {"null?", null_fn},
  {"pair?", pair_fn},
  {"abs", abs_fn},
  {"sin", sin_fn},
  {"cos", cos_fn},
  {"log", log_fn},
  {"max", max_fn},
  {"min", min_fn},
  {"expt", exp},
  {"quotient", quotient},
  {"remainder", remainder},
  {"sqrt", sqrt_fn},
  {"newline", new_line},
  {"display", display_final},
  {"length", list_len},
  {"append", append},
  {"error", error_fn}
};

const vector<pair<string, Obj>>
consts = {
  {"true", true},
  {"false", false},
  {"#t", true},
  {"#f", false},
  {"nil", nullptr}
};

}
