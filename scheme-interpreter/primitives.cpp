#include "interface.hpp"
using namespace std;

namespace scheme {

void 
display(const bool);

void 
display(const double);

void 
display(const symbol&);

void 
display(const string&);

void 
display(const nullptr_t);

void 
display(cons*);

void 
display(const bool b) {
  cout << (b ? "#t" : "#f");
}

void 
display(const double n) {
  cout << n;
}

void 
display(const symbol& s) {
  cout << s.name;
}

void 
display(const string& w) {
  cout << "\"";
  cout << w;
  cout << "\"";
}

void 
display(const nullptr_t x) {}

void 
display(const procedure *p) {
  cout << "procedure at " << p;
}

void 
display(const primitive *p) {
  cout << "primitive at " << p;
}

struct {
  template<typename T>
  void 
  operator()(T x) {
    display(x);
  }
} display_overload;

sc_obj
display_iter(const sc_obj obj, const bool head) {
  if (is_pair(obj)) {
    if (!head)
      cout << " ";
    let c = get<cons*>(obj);
    visit(display_overload, c->car);
    return display_iter(c->cdr, 0);
  }
  else {
    return obj;
  }
}

void 
display(cons *const c) {
  cout << "(";
  sc_obj obj = \
  display_iter(c, 1);

  if (!is_null(obj)) {
    cout << " . ";
    visit(display_overload, obj);
  }
  cout << ")";
}

void 
display(const sc_obj obj) {
  visit(display_overload, obj);
}

namespace prim_env {
  sc_obj
  display_final(const vector<sc_obj>& args) {
    for (let arg : args) {
      display(arg);
      cout << "\n";
    }
    return symbol("ok");
  }

  sc_obj
  add(const vector<sc_obj>& args) {
    double ret = 0.0;
    for (let arg : args) {
      ret += get<double>(arg);
    }
    return ret;
  }

  sc_obj
  sub(const vector<sc_obj>& args) {
    if (args.size() == 1)
      return -get<double>(args[0]);
    else
      return get<double>(args[0]) - get<double>(args[1]);   
  }

  sc_obj
  mul(const vector<sc_obj>& args) {
    double ret = 1.0;
    for (let arg : args) {
      ret *= get<double>(arg);
    }
    return ret;
  }

  sc_obj
  div(const vector<sc_obj>& args) {
    return get<double>(args[0]) / get<double>(args[1]);
  }

  sc_obj 
  exp(const vector<sc_obj>& args) {
    return pow(get<double>(args[0]), get<double>(args[1]));
  }

  sc_obj 
  remainder(const vector<sc_obj>& args) {
    return fmod(get<double>(args[0]), get<double>(args[1]));
  }

  sc_obj
  lt(const vector<sc_obj>& args) {
    return get<double>(args[0]) < get<double>(args[1]);
  }

  sc_obj
  gt(const vector<sc_obj>& args) {
    return get<double>(args[0]) > get<double>(args[1]);
  }

  sc_obj 
  eq(const vector<sc_obj>& args) {
    return abs(get<double>(args[0]) - get<double>(args[1])) < precision;
  }

  sc_obj 
  le(const vector<sc_obj>& args) {
    return get<double>(args[0]) <= get<double>(args[1]);
  }

  sc_obj
  ge(const vector<sc_obj>& args) {
    return get<double>(args[0]) >= get<double>(args[1]);
  }

  sc_obj
  neq(const vector<sc_obj>& args) {
    return !get<bool>(eq(args));
  }

  sc_obj
  is_equal(const vector<sc_obj>& args) {
    return args[0] == args[1];
  }

  sc_obj
  car(const vector<sc_obj>& args) {
    return get<cons*>(args[0])->car;
  }

  sc_obj
  cdr(const vector<sc_obj>& args) {
    return get<cons*>(args[0])->cdr;
  }

  sc_obj
  cons_fn(const vector<sc_obj>& args) {
    return new cons(args[0], args[1]);
  }

  sc_obj
  list_fn(const vector<sc_obj>& args) {
    sc_obj ret = nil;
    for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
      ret = new cons(*curr, ret);
    }
    return ret;
  }

  sc_obj
  null_fn(const vector<sc_obj>& args) {
    return holds_alternative<nullptr_t>(args[0]);
  }

  sc_obj
  pair_fn(const vector<sc_obj>& args) {
    return is_pair(args[0]);
  }

  sc_obj
  id(const vector<sc_obj>& args) {
    return args[0];
  }

  sc_obj
  inc(const vector<sc_obj>& args) {
    return 1 + get<double>(args[0]);
  };

  sc_obj
  dec(const vector<sc_obj>& args) {
    return -1 + get<double>(args[0]);
  };

  sc_obj
  avg(const vector<sc_obj>& args) {
    return get<double>(add(args)) / args.size();
  };

  sc_obj
  sq(const vector<sc_obj>& args) {
    return get<double>(args[0]) * get<double>(args[0]);
  }

  sc_obj
  abs_fn(const vector<sc_obj>& args) {
    return abs(get<double>(args[0]));
  }

  sc_obj
  sin_fn(const vector<sc_obj>& args) {
    return sin(get<double>(args[0]));
  }

  sc_obj
  cos_fn(const vector<sc_obj>& args) {
    return cos(get<double>(args[0]));
  }

  sc_obj
  log_fn(const vector<sc_obj>& args) {
    return log(get<double>(args[0]));
  }

  sc_obj
  max_fn(const vector<sc_obj>& args) {
    double ret = -INFINITY;
    for (let arg : args)
      ret = max(ret, get<double>(arg));
    return ret;
  }

  sc_obj
  min_fn(const vector<sc_obj>& args) {
    double ret = -INFINITY;
    for (let arg : args)
      ret = min(ret, get<double>(arg));
    return ret;
  }

  sc_obj
  quotient(const vector<sc_obj>& args) {
    return static_cast<double>(
      static_cast<int>(
        get<double>(args[0]) / 
        get<double>(args[1])));
  }

  sc_obj
  sqrt_fn(const vector<sc_obj>& args) {
    return sqrt(get<double>(args[0]));
  }

  sc_obj
  not_fn(const vector<sc_obj>& args) {
    return !get<bool>(args[0]);
  }

  sc_obj
  new_line(const vector<sc_obj>& args) {
    cout << "\n";
    return true;
  }

  sc_obj
  list_len(const vector<sc_obj>& args) {
    double ret = 0;
    auto curr = args[0];
    while (is_pair(curr)) {
      ret++;
      curr = get<cons*>(curr)->cdr;
    }
    return ret;
  }

  sc_obj
  append_rec(const sc_obj list1, const sc_obj list2) {
    if (is_pair(list1)) {
      return new cons(
        get<cons*>(list1)->car,
        append_rec(get<cons*>(list1)->cdr, list2)
      );
    }
    else {
      return list2;
    }
  }

  sc_obj
  append(const vector<sc_obj>& args) {
    return append_rec(args[0], args[1]);
  }
}

using namespace prim_env;

const vector<pair<string, function<sc_obj(const vector<sc_obj>&)>>> 
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
  {"car", car},
  {"cdr", cdr},
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
  {"append", append}
};

}