#include "primitives.hpp"
#include <cmath>

namespace Scheme {

static Obj
display_list(const vector<Obj>& args) {
  for (const auto& arg : args) {
    display(arg);
  }
  return Symbol("ok");
}

static Obj
add(const vector<Obj>& args) {
  double ret = 0.0;
  for (const auto& arg : args) {
    ret += get<double>(arg);
  }
  return ret;
}

static Obj
sub(const vector<Obj>& args) {
  if (args.size() == 1)
    return -get<double>(args[0]);
  else
    return get<double>(args[0]) - get<double>(args[1]);   
}

static Obj
mul(const vector<Obj>& args) {
  double ret = 1.0;
  for (const auto& arg : args) {
    ret *= get<double>(arg);
  }
  return ret;
}

static Obj
div(const vector<Obj>& args) {
  return get<double>(args[0]) / get<double>(args[1]);
}

static Obj 
exp(const vector<Obj>& args) {
  return pow(get<double>(args[0]), get<double>(args[1]));
}

static Obj 
remainder(const vector<Obj>& args) {
  return fmod(get<double>(args[0]), get<double>(args[1]));
}

static Obj
lt(const vector<Obj>& args) {
  return get<double>(args[0]) < get<double>(args[1]);
}

static Obj
gt(const vector<Obj>& args) {
  return get<double>(args[0]) > get<double>(args[1]);
}

static Obj 
eq(const vector<Obj>& args) {
  return abs(get<double>(args[0]) - get<double>(args[1])) < precision;
}

static Obj 
le(const vector<Obj>& args) {
  return get<double>(args[0]) <= get<double>(args[1]);
}

static Obj
ge(const vector<Obj>& args) {
  return get<double>(args[0]) >= get<double>(args[1]);
}

static Obj
neq(const vector<Obj>& args) {
  return !get<bool>(eq(args));
}

static Obj
is_equal(const vector<Obj>& args) {
  return args[0] == args[1];
}

static Obj
cons_fn(const vector<Obj>& args) {
  return new Cons(args[0], args[1]);
}

static Obj
list_fn(const vector<Obj>& args) {
  Obj ret = nullptr;
  for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
    ret = new Cons(*curr, ret);
  }
  return ret;
}

static Obj
null_fn(const vector<Obj>& args) {
  return holds_alternative<nullptr_t>(args[0]);
}

static Obj
pair_fn(const vector<Obj>& args) {
  return is_pair(args[0]);
}

static Obj
id(const vector<Obj>& args) {
  return args[0];
}

static Obj
inc(const vector<Obj>& args) {
  return 1 + get<double>(args[0]);
};

static Obj
dec(const vector<Obj>& args) {
  return -1 + get<double>(args[0]);
};

static Obj
avg(const vector<Obj>& args) {
  return get<double>(add(args)) / args.size();
};

static Obj
sq(const vector<Obj>& args) {
  return get<double>(args[0]) * get<double>(args[0]);
}

static Obj
abs_fn(const vector<Obj>& args) {
  return std::abs(get<double>(args[0]));
}

static Obj
sin_fn(const vector<Obj>& args) {
  return sin(get<double>(args[0]));
}

static Obj
cos_fn(const vector<Obj>& args) {
  return cos(get<double>(args[0]));
}

static Obj
log_fn(const vector<Obj>& args) {
  return log(get<double>(args[0]));
}

static Obj
max_fn(const vector<Obj>& args) {
  double ret = -INFINITY;
  for (const auto& arg : args)
    ret = std::max(ret, get<double>(arg));
  return ret;
}

static Obj
min_fn(const vector<Obj>& args) {
  double ret = -INFINITY;
  for (const auto& arg : args)
    ret = std::min(ret, get<double>(arg));
  return ret;
}

static Obj
quotient(const vector<Obj>& args) {
  return static_cast<double>(
    static_cast<int>(
      get<double>(args[0]) / 
      get<double>(args[1])));
}

static Obj
sqrt_fn(const vector<Obj>& args) {
  return sqrt(get<double>(args[0]));
}

static Obj
not_fn(const vector<Obj>& args) {
  return !get<bool>(args[0]);
}

static Obj
new_line(const vector<Obj>& args) {
  cout << "\n";
  return true;
}

static Obj
list_len(const vector<Obj>& args) {
  double ret = 0;
  auto curr = args[0];
  while (is_pair(curr)) {
    ret++;
    curr = get<Cons*>(curr)->cdr;
  }
  return ret;
}

static Obj
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

static Obj
append(const vector<Obj>& args) {
  return append_rec(args[0], args[1]);
}

static Obj
error_fn(const vector<Obj>& args) {
  cout << "ERROR: ";
  for (auto obj : args) {
    display(obj);
    cout << " ";
  }
  return nullptr;
}

vector<std::pair<string, Obj(*)(const vector<Obj>&)>> 
get_primitive_functions() {
  return {
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
    {"display", display_list},
    {"length", list_len},
    {"append", append},
    {"error", error_fn}
  };
}

vector<std::pair<string, Obj>>
get_consts() {
  return {
    {"true", true},
    {"false", false},
    {"#t", true},
    {"#f", false},
    {"nil", nullptr}
  };
}

}
