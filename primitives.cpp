#include "types.hpp"
#include "environment.hpp"
#include "primitives.hpp"
#include "evaluation.hpp"
#include "interpreter.hpp"
#include <iostream>
#include <cmath>
#include <climits>

namespace Scheme {

constexpr double precision = 1e-9;
constexpr int MAX_ARGS = INT_MAX;

template<typename T>
static void
assert_obj_type(const Obj& obj, const std::string& type) {
  if (!std::holds_alternative<T>(obj)) {
    throw std::runtime_error("incorrect type for " + stringify(obj) + ", expected " + type);
  }
}

template<typename T>
static void
assert_vec_type(const ArgList& args, const std::string& type) {
  for (int i = 0; i < args.size(); i++) {
    assert_obj_type<T>(args[i], type);
  }
}

static void
assert_callable(const Obj& obj) {
  if (!is_procedure(obj) && !is_primitive(obj)) {
    throw std::runtime_error("incorrect type for " + stringify(obj) + ", exprected procedure");
  }
}

static void
assert_arg_count(const ArgList& args, const int lb, const int rb) {
  if (!(lb <= args.size() && args.size() <= rb)) {
    if (rb == MAX_ARGS) {
      throw std::runtime_error("incorrect number of arguments: expected at least " + std::to_string(lb));
    }
    else if (lb == rb) {
      throw std::runtime_error("incorrect number of arguments: expected " + std::to_string(lb));
    }
    else {
      throw std::runtime_error("incorrect number of arguments: expected between " + std::to_string(lb) + " and " + std::to_string(rb));
    }
  }
}

static Obj
display(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  std::cout << stringify(args[0]);
  return Void {};
}

static Obj
add(const ArgList& args, Interpreter& interp) {
  assert_vec_type<double>(args, "number");
  double ret = 0.0;
  for (const auto& arg : args) {
    ret += as_number(arg);
  }
  return ret;
}

static Obj
sub(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  if (args.size() == 1) {
    return -as_number(args[0]);
  }
  else {
    double ret = as_number(args[0]);
    for (int i = 1; i < args.size(); i++) {
      ret -= as_number(args[i]);
    }
    return ret;
  }
}

static Obj
mul(const ArgList& args, Interpreter& interp) {
  assert_vec_type<double>(args, "number");
  double ret = 1.0;
  for (const auto& arg : args) {
    ret *= as_number(arg);
  }
  return ret;
}

static Obj
div(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  if (args.size() == 1) {
    return 1.0 / as_number(args[0]);
  }
  else {
    double ret = as_number(args[0]);
    for (int i = 1; i < args.size(); i++) {
      ret /= as_number(args[i]);
    }
    return ret;
  }
}

static Obj
quotient(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_vec_type<double>(args, "number");
  return static_cast<double>(
    static_cast<int>(
      as_number(args[0]) / 
      as_number(args[1])));
}

static Obj 
remainder(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_vec_type<double>(args, "number");
  return fmod(as_number(args[0]), as_number(args[1]));
}

static Obj
expt(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_vec_type<double>(args, "number");
  return std::pow(as_number(args[0]), as_number(args[1]));
}

template<class Comp>
static bool
check_comp(const ArgList& args, Comp comp) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  for (int i = 1; i < args.size(); i++) {
    if (!comp(as_number(args[i - 1]), as_number(args[i]))) {
      return false;
    }
  }
  return true;
}

static Obj
lt(const ArgList& args, Interpreter& interp) {
  return check_comp(args, std::less<double>());
}

static Obj
gt(const ArgList& args, Interpreter& interp) {
  return check_comp(args, std::greater<double>());
}

static Obj 
eq(const ArgList& args, Interpreter& interp) {
  return check_comp(args, std::equal_to<double>());
}

static Obj 
le(const ArgList& args, Interpreter& interp) {
  return check_comp(args, std::less_equal<double>());
}

static Obj
ge(const ArgList& args, Interpreter& interp) {
  return check_comp(args, std::greater_equal<double>());
}

static Obj
abs_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::abs(as_number(args[0]));
}

static Obj
sqrt_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::sqrt(as_number(args[0]));
}

static Obj
sin_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return sin(as_number(args[0]));
}

static Obj
cos_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return cos(as_number(args[0]));
}

static Obj
log_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return log(as_number(args[0]));
}

static Obj
max_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  double ret = -INFINITY;
  for (const auto& arg : args)
    ret = std::max(ret, as_number(arg));
  return ret;
}

static Obj
min_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<double>(args, "number");
  double ret = INFINITY;
  for (const auto& arg : args)
    ret = std::min(ret, as_number(arg));
  return ret;
}

static Obj
is_even(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return (bool) !(1 & static_cast<int>(as_number(args[0])));
}

static Obj
is_odd(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return (bool) (1 & static_cast<int>(as_number(args[0])));
}

static Obj
ceil_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::ceil(as_number(args[0]));
}

static Obj
floor_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::floor(as_number(args[0]));
}

static Obj
round_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return std::round(as_number(args[0]));
}

static Obj
not_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_false(args[0]);
}

static Obj
is_eq(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  return args[0] == args[1];
}

static bool
eq_list(const ArgList& args) {
  Obj l0 = args[0];
  Obj l1 = args[1];
  while (is_pair(l0) && is_pair(l1)) {
    if (as_pair(l0)->car != as_pair(l1)->car) {
      return false;
    }
    l0 = as_pair(l0)->cdr;
    l1 = as_pair(l1)->cdr;
  }
  return l0 == l1;
}

static Obj
is_equal(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  if (is_pair(args[0])) {
    return eq_list(args);
  }
  else {
    return is_eq(args, interp);
  }
}

static Obj
cons_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  return interp.alloc.make_cons(args[0], args[1]);
}

static Obj
list_fn(const ArgList& args, Interpreter& interp) {
  Obj ret = nullptr;
  for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
    ret = interp.alloc.make_cons(*curr, ret);
  }
  return ret;
}

static Obj
is_null(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_null(args[0]);
}

static Obj
is_boolean(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_bool(args[0]);
}

static Obj
is_number(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_number(args[0]);
}

static Obj
is_pair(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_pair(args[0]);
}

static Obj
is_symbol(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_symbol(args[0]);
}

static Obj
is_string(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_string(args[0]);
}

static Obj
is_procedure(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_procedure(args[0]) || is_primitive(args[0]);
}

static Obj
new_line(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 0, 0);
  std::cout << "\n";
  return Void {};
}

static Obj
set_car(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_obj_type<Cons*>(args[0], "list");
  as_pair(args[0])->car = args[1];
  return Void {};
}

static Obj
set_cdr(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_obj_type<Cons*>(args[0], "list");
  as_pair(args[0])->cdr = args[1];
  return Void {};
}

static Obj
list_len(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<Cons*>(args[0], "list");
  double ret = 0;
  auto curr = args[0];
  while (is_pair(curr)) {
    ret++;
    curr = as_pair(curr)->cdr;
  }
  return ret;
}

static Obj
list_ref(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_obj_type<Cons*>(args[0], "list");
  assert_obj_type<double>(args[1], "number");
  Obj ls = args[0];
  int i = 0;
  const auto n = as_number(args[1]);
  while (is_pair(ls) && i < n) {
    ls = as_pair(ls)->cdr;
    i++;
  }
  if (i == n) {
    return ls;
  }
  else {
    throw std::runtime_error("longer list expected");
  }
}

static Obj
append_rec(const Obj& list1, const Obj& list2, Interpreter& interp) {
  if (is_pair(list1)) {
    return interp.alloc.make_cons(
      as_pair(list1)->car,
      append_rec(as_pair(list1)->cdr, list2, interp)
    );
  }
  else {
    return list2;
  }
}

static Obj
append(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, MAX_ARGS);
  assert_vec_type<Cons*>(args, "list");
  Obj ret = nullptr;
  for (int i = args.size() - 1; i >= 0; i--) {
    ret = append_rec(args[i], ret, interp);
  }
  return ret;
}

static Obj 
map_rec(Obj& fn, Obj& obj, Interpreter& interp) {
  if (!is_pair(obj)) {
    return obj;
  }
  else {
    auto ls = as_pair(obj);
    return interp.alloc.make_cons(
      as_obj(apply(fn, ArgList({ls->car}), interp)), 
      map_rec(fn, ls->cdr, interp)
    );
  }
}

static Obj
map_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_callable(args[0]);
  assert_obj_type<Cons*>(args[1], "list");
  auto fn = args[0];
  auto ls = args[1];
  auto ret = map_rec(fn, ls, interp);
  return ret;
}

static Obj 
filter_rec(Obj& fn, Obj& obj, Interpreter& interp) {
  if (!is_pair(obj)) {
    return obj;
  }
  else {
    auto ls = as_pair(obj);
    auto rest = filter_rec(fn, ls->cdr, interp);
    if (is_true(as_obj(apply(fn, ArgList({ls->car}), interp)))) {
      return interp.alloc.make_cons(ls->car, rest);
    }
    else {
      return rest;
    }
  }
}

static Obj
filter_fn(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_callable(args[0]);
  assert_obj_type<Cons*>(args[1], "list");
  auto fn = args[0];
  auto ls = args[1];
  return filter_rec(fn, ls, interp);
}

static Obj
error_fn(const ArgList& args, Interpreter& interp) {
  std::cerr << "ERROR: ";
  for (auto obj : args) {
    std::cerr << stringify(obj);
    std::cerr << " ";
  }
  return Void {};
}

std::vector<std::pair<std::string, Obj(*)(const ArgList&, Interpreter&)>> 
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
    {"set-car!", set_car},
    {"set-cdr!", set_cdr},
    {"length", list_len},
    {"list-ref", list_ref},
    {"append", append},
    {"map", map_fn},
    {"filter", filter_fn},
    {"error", error_fn}
  };
}

std::vector<std::pair<std::string, Obj>>
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
