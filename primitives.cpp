#include "types.hpp"
#include "environment.hpp"
#include "stringify.hpp"
#include "primitives.hpp"
#include "evaluation.hpp"
#include <cmath>
#include <climits>

namespace Scheme {

constexpr double precision = 1e-9;
constexpr int MAX_ARGS = INT_MAX;

template<typename T>
static void
assert_obj_type(const Obj& obj, const string& type) {
  if (!std::holds_alternative<T>(obj)) {
    throw runtime_error("incorrect type for " + stringify(obj) + ", expected " + type);
  }
}

template<typename T>
static void
assert_vec_type(const vector<Obj>& args, const string& type) {
  for (int i = 0; i < args.size(); i++) {
    assert_obj_type<T>(args[i], type);
  }
}

static void
assert_callable(const Obj& obj) {
  if (!is_procedure(obj) && !is_primitive(obj)) {
    throw runtime_error("incorrect type for " + stringify(obj) + ", exprected procedure");
  }
}

static void
assert_arg_count(const vector<Obj>& args, const int lb, const int rb) {
  if (!(lb <= args.size() && args.size() <= rb)) {
    if (rb == MAX_ARGS) {
      throw runtime_error("incorrect number of arguments: expected at least " + std::to_string(lb));
    }
    else if (lb == rb) {
      throw runtime_error("incorrect number of arguments: expected " + std::to_string(lb));
    }
    else {
      throw runtime_error("incorrect number of arguments: expected between " + std::to_string(lb) + " and " + std::to_string(rb));
    }
  }
}

static Obj
display(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  cout << stringify(args[0]);
  return Void {};
}

static Obj
add(const vector<Obj>& args) {
  assert_vec_type<double>(args, "number");
  double ret = 0.0;
  for (const auto& arg : args) {
    ret += get<double>(arg);
  }
  return ret;
}

static Obj
sub(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  if (args.size() == 1) {
    return -get<double>(args[0]);
  }
  else {
    double ret = get<double>(args[0]);
    for (int i = 1; i < args.size(); i++) {
      ret -= get<double>(args[i]);
    }
    return ret;
  }
}

static Obj
mul(const vector<Obj>& args) {
  assert_vec_type<double>(args, "number");
  double ret = 1.0;
  for (const auto& arg : args) {
    ret *= get<double>(arg);
  }
  return ret;
}

static Obj
div(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  if (args.size() == 1) {
    return 1.0 / get<double>(args[0]);
  }
  else {
    double ret = get<double>(args[0]);
    for (int i = 1; i < args.size(); i++) {
      ret /= get<double>(args[i]);
    }
    return ret;
  }
}

static Obj
quotient(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  assert_vec_type<double>(args, "number");
  return static_cast<double>(
    static_cast<int>(
      get<double>(args[0]) / 
      get<double>(args[1])));
}

static Obj 
remainder(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  assert_vec_type<double>(args, "number");
  return fmod(get<double>(args[0]), get<double>(args[1]));
}

static Obj
expt(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  assert_vec_type<double>(args, "number");
  return std::pow(get<double>(args[0]), get<double>(args[1]));
}

template<class Comp>
static bool
check_comp(const vector<Obj>& args, Comp comp) {
  for (int i = 1; i < args.size(); i++) {
    if (!comp(get<double>(args[i - 1]), get<double>(args[i]))) {
      return false;
    }
  }
  return true;
}

static Obj
lt(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  return check_comp(args, std::less<double>());
}

static Obj
gt(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  return check_comp(args, std::greater<double>());
}

static Obj 
eq(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  return check_comp(args, std::equal_to<double>());
}

static Obj 
le(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  return check_comp(args, std::less_equal<double>());
}

static Obj
ge(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  return check_comp(args, std::greater_equal<double>());
}

static Obj
abs_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::abs(get<double>(args[0]));
}

static Obj
sqrt_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::sqrt(get<double>(args[0]));
}

static Obj
sin_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return sin(get<double>(args[0]));
}

static Obj
cos_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return cos(get<double>(args[0]));
}

static Obj
log_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return log(get<double>(args[0]));
}

static Obj
max_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  double ret = -INFINITY;
  for (const auto& arg : args)
    ret = std::max(ret, get<double>(arg));
  return ret;
}

static Obj
min_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  double ret = INFINITY;
  for (const auto& arg : args)
    ret = std::min(ret, get<double>(arg));
  return ret;
}

static Obj
is_even(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return (bool) !(1 & static_cast<int>(get<double>(args[0])));
}

static Obj
is_odd(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return (bool) (1 & static_cast<int>(get<double>(args[0])));
}

static Obj
ceil_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::ceil(get<double>(args[0]));
}

static Obj
floor_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::floor(get<double>(args[0]));
}

static Obj
round_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::round(get<double>(args[0]));
}

static Obj
not_fn(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_false(args[0]);
}

static Obj
is_eq(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  return args[0] == args[1];
}

static bool
eq_list(const vector<Obj>& args) {
  Obj l0 = args[0];
  Obj l1 = args[1];
  while (is_pair(l0) && std::holds_alternative<Cons*>(l1)) {
    if (get<Cons*>(l0)->car != get<Cons*>(l1)->car) {
      return false;
    }
    l0 = get<Cons*>(l0)->cdr;
    l1 = get<Cons*>(l1)->cdr;
  }
  return l0 == l1;
}

static Obj
is_equal(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  if (is_pair(args[0])) {
    return eq_list(args);
  }
  else {
    return is_eq(args);
  }
}

static Obj
cons_fn(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
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
is_null(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_null(args[0]);
}

static Obj
is_boolean(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_bool(args[0]);
}

static Obj
is_number(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_number(args[0]);
}

static Obj
is_pair(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_pair(args[0]);
}

static Obj
is_symbol(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_symbol(args[0]);
}

static Obj
is_string(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_string(args[0]);
}

static Obj
is_procedure(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  return is_procedure(args[0]) || is_primitive(args[0]);
}

static Obj
new_line(const vector<Obj>& args) {
  assert_arg_count(args, 0, 0);
  cout << "\n";
  return Void {};
}

static Obj
list_len(const vector<Obj>& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<Cons*>(args[0], "list");
  double ret = 0;
  auto curr = args[0];
  while (is_pair(curr)) {
    ret++;
    curr = get<Cons*>(curr)->cdr;
  }
  return ret;
}

static Obj
list_ref(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  assert_obj_type<Cons*>(args[0], "list");
  assert_obj_type<double>(args[1], "number");
  Obj ls = args[0];
  int i = 0;
  const auto n = get<double>(args[1]);
  while (is_pair(ls) && i < n) {
    ls = get<Cons*>(ls)->cdr;
    i++;
  }
  if (i == n) {
    return ls;
  }
  else {
    throw runtime_error("longer list expected");
  }
}

static Obj
append_rec(Obj list1, Obj list2) {
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
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<Cons*>(args, "list");
  Obj ret = nullptr;
  for (int i = args.size() - 1; i >= 0; i--) {
    ret = append_rec(args[i], ret);
  }
  return ret;
}

static Obj 
map_rec(Obj& fn, Obj& obj) {
  if (!is_pair(obj)) {
    return obj;
  }
  else {
    auto ls = get<Cons*>(obj);
    return new Cons(
      get<Obj>(apply(fn, vector<Obj>({ls->car}))), 
      map_rec(fn, ls->cdr)
    );
  }
}

static Obj
map_fn(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  assert_callable(args[0]);
  assert_obj_type<Cons*>(args[1], "list");
  auto fn = args[0];
  auto ls = args[1];
  auto ret = map_rec(fn, ls);
  return ret;
}

static Obj 
filter_rec(Obj& fn, Obj& obj) {
  if (!is_pair(obj)) {
    return obj;
  }
  else {
    auto ls = get<Cons*>(obj);
    auto rest = filter_rec(fn, ls->cdr);
    if (is_true(get<Obj>(apply(fn, vector<Obj>({ls->car}))))) {
      return new Cons(ls->car, rest);
    }
    else {
      return rest;
    }
  }
}

static Obj
filter_fn(const vector<Obj>& args) {
  assert_arg_count(args, 2, 2);
  assert_callable(args[0]);
  assert_obj_type<Cons*>(args[1], "list");
  auto fn = args[0];
  auto ls = args[1];
  return filter_rec(fn, ls);
}

static Obj
error_fn(const vector<Obj>& args) {
  cerr << "ERROR: ";
  for (auto obj : args) {
    cerr << stringify(obj);
    cerr << " ";
  }
  return Void {};
}

vector<std::pair<string, Obj(*)(const vector<Obj>&)>> 
get_primitive_functions() {
  return {
    {"+", add},
    {"-", sub},
    {"*", mul},
    {"/", div},
    {"<", lt},
    {">", gt},
    {"=", eq},
    {"<=", le},
    {">=", ge},
    {"eq?", is_eq},
    {"equal?", is_equal},
    {"not", not_fn},
    {"cons", cons_fn},
    {"list", list_fn},
    {"null?", is_null},
    {"boolean?", is_boolean},
    {"number?", is_number},
    {"symbol?", is_symbol},
    {"string?", is_string},
    {"pair?", is_pair},
    {"procedure?", is_procedure},
    {"abs", abs_fn},
    {"sqrt", sqrt_fn},
    {"sin", sin_fn},
    {"cos", cos_fn},
    {"log", log_fn},
    {"max", max_fn},
    {"min", min_fn},
    {"even?", is_even},
    {"odd?", is_odd},
    {"ceil", ceil_fn},
    {"floor", floor_fn},
    {"round", round_fn},
    {"expt", expt},
    {"quotient", quotient},
    {"remainder", remainder},
    {"newline", new_line},
    {"display", display},
    {"length", list_len},
    {"list-ref", list_ref},
    {"append", append},
    {"map", map_fn},
    {"filter", filter_fn},
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
