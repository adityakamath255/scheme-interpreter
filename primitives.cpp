#include "types.hpp"
#include "environment.hpp"
#include "primitives.hpp"
#include "expressions.hpp"
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
  for (size_t i = 0; i < args.size(); i++) {
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

static void
assert_numbers(const ArgList& args, int lb, int rb) {
  assert_arg_count(args, lb, rb);
  assert_vec_type<double>(args, "number");
}

static double
get_single_number(const ArgList& args) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<double>(args[0], "number");
  return as_number(args[0]);
}

static Obj
display(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  std::cout << stringify(args[0]);
  return Void {};
}

static Obj
add(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 0, MAX_ARGS);
  double ret = 0.0;
  for (const auto& arg : args) {
    ret += as_number(arg);
  }
  return ret;
}

static Obj
sub(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 1, MAX_ARGS);
  if (args.size() == 1) {
    return -as_number(args[0]);
  }
  else {
    double ret = as_number(args[0]);
    for (size_t i = 1; i < args.size(); i++) {
      ret -= as_number(args[i]);
    }
    return ret;
  }
}

static Obj
mul(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 0, MAX_ARGS);
  double ret = 1.0;
  for (const auto& arg : args) {
    ret *= as_number(arg);
  }
  return ret;
}

static Obj
div(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 1, MAX_ARGS);
  if (args.size() == 1) {
    return 1.0 / as_number(args[0]);
  }
  else {
    double ret = as_number(args[0]);
    for (size_t i = 1; i < args.size(); i++) {
      ret /= as_number(args[i]);
    }
    return ret;
  }
}

static Obj
quotient(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 2, 2);
  return static_cast<double>(
    static_cast<int>(
      as_number(args[0]) / 
      as_number(args[1])));
}

static Obj 
remainder(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 2, 2);
  return fmod(as_number(args[0]), as_number(args[1]));
}

static Obj
expt(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 2, 2);
  return std::pow(as_number(args[0]), as_number(args[1]));
}

template<class Comp>
static bool
check_comp(const ArgList& args, Comp comp) {
  assert_numbers(args, 1, MAX_ARGS);
  for (size_t i = 1; i < args.size(); i++) {
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
abs_prim(const ArgList& args, Interpreter& interp) {
  return std::abs(get_single_number(args));
}

static Obj
sqrt_prim(const ArgList& args, Interpreter& interp) {
  return std::sqrt(get_single_number(args));
}

static Obj
sin_prim(const ArgList& args, Interpreter& interp) {
  return std::sin(get_single_number(args));
}

static Obj
cos_prim(const ArgList& args, Interpreter& interp) {
  return std::cos(get_single_number(args));
}

static Obj
log_prim(const ArgList& args, Interpreter& interp) {
  return std::log(get_single_number(args));
}

static Obj
max_prim(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 1, MAX_ARGS);
  double ret = -INFINITY;
  for (const auto& arg : args)
    ret = std::max(ret, as_number(arg));
  return ret;
}

static Obj
min_prim(const ArgList& args, Interpreter& interp) {
  assert_numbers(args, 1, MAX_ARGS);
  double ret = INFINITY;
  for (const auto& arg : args)
    ret = std::min(ret, as_number(arg));
  return ret;
}

static Obj
is_even(const ArgList& args, Interpreter& interp) {
  return (bool) !(1 & static_cast<int>(get_single_number(args)));
}

static Obj
is_odd(const ArgList& args, Interpreter& interp) {
  return (bool) (1 & static_cast<int>(get_single_number(args)));
}

static Obj
ceil_prim(const ArgList& args, Interpreter& interp) {
  return std::ceil(get_single_number(args));
}

static Obj
floor_prim(const ArgList& args, Interpreter& interp) {
  return std::floor(get_single_number(args));
}

static Obj
round_prim(const ArgList& args, Interpreter& interp) {
  return std::round(get_single_number(args));
}

static Obj
not_prim(const ArgList& args, Interpreter& interp) {
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
  else if (is_vector(args[0])) {
    return as_vector(args[0])->data == as_vector(args[1])->data;
  }
  else {
    return is_eq(args, interp);
  }
}

static Obj
cons_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  return interp.alloc.make<Cons>(args[0], args[1]);
}

static Obj
list_prim(const ArgList& args, Interpreter& interp) {
  Obj ret = nullptr;
  for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
    ret = interp.alloc.make<Cons>(*curr, ret);
  }
  return ret;
}

static Obj
is_null_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_null(args[0]);
}

static Obj
is_boolean_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_bool(args[0]);
}

static Obj
is_number_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_number(args[0]);
}

static Obj
is_pair_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_pair(args[0]);
}

static Obj
is_vector_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_vector(args[0]);
}

static Obj
is_symbol_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_symbol(args[0]);
}

static Obj
is_string_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  return is_string(args[0]);
}

static Obj
is_procedure_prim(const ArgList& args, Interpreter& interp) {
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
  if (is_null(args[0])) {
    return 0.0;
  }
  else {
    assert_obj_type<Cons*>(args[0], "list");
    double ret = 0;
    auto curr = args[0];
    while (is_pair(curr)) {
      ret++;
      curr = as_pair(curr)->cdr;
    }
    return ret;
  }
}

static Obj
list_ref(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_obj_type<Cons*>(args[0], "list");
  assert_obj_type<double>(args[1], "number");
  if (as_number(args[1]) < 0) {
    throw std::runtime_error("list index cannot be negative");
  }
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
    return interp.alloc.make<Cons>(
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
  for (size_t i = args.size() - 1; i >= 0; i--) {
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
    return interp.alloc.make<Cons>(
      as_obj(apply(fn, ArgList({ls->car}), interp)), 
      map_rec(fn, ls->cdr, interp)
    );
  }
}

static Obj
map_prim(const ArgList& args, Interpreter& interp) {
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
      return interp.alloc.make<Cons>(ls->car, rest);
    }
    else {
      return rest;
    }
  }
}

static Obj
filter_prim(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_callable(args[0]);
  assert_obj_type<Cons*>(args[1], "list");
  auto fn = args[0];
  auto ls = args[1];
  return filter_rec(fn, ls, interp);
}

static Obj
make_vector(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 2);
  assert_obj_type<double>(args[0], "number");
  const size_t sz = as_number(args[0]);
  if (sz < 0) {
    throw std::runtime_error("vector size cannot be negative");
  }
  if (args.size() == 2) {
    return interp.alloc.make<Vector>(
      std::vector<Obj>(sz, args[1])
    );
  }
  else {
    return interp.alloc.make<Vector>(
      std::vector<Obj>(sz, Obj {(double) 0})
    );
  }
}

static Obj
vector_ref(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 2, 2);
  assert_obj_type<Vector*>(args[0], "vector");
  assert_obj_type<double>(args[1], "number");
  const size_t index = as_number(args[1]);
  if (index < 0) {
    throw std::runtime_error("vector index cannot be negative");
  }
  std::vector<Obj>& data = as_vector(args[0])->data;
  if (index > data.size()) {
    throw std::runtime_error("vector index out of range");
  }
  return data[index]; 
}

static Obj
vector_set(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 3, 3);
  assert_obj_type<Vector*>(args[0], "vector");
  assert_obj_type<double>(args[1], "number");
  const size_t index = as_number(args[1]);
  if (index < 0) {
    throw std::runtime_error("vector index cannot be negative");
  }
  std::vector<Obj>& data = as_vector(args[0])->data;
  if (index > data.size()) {
    throw std::runtime_error("vector index out of range");
  }
  data[index] = args[2];
  return Void {};
}

static Obj
vector_length(const ArgList& args, Interpreter& interp) {
  assert_arg_count(args, 1, 1);
  assert_obj_type<Vector*>(args[0], "vector");
  return (double) as_vector(args[0])->data.size();
}

static Obj
error_prim(const ArgList& args, Interpreter& interp) {
  std::ostringstream message;
  message << "ERROR: ";
  for (auto obj : args) {
    message << stringify(obj);
    message << " ";
  }
  throw std::runtime_error(message.str());
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
    {"not", not_prim},
    {"cons", cons_prim},
    {"list", list_prim},
    {"null?", is_null_prim},
    {"boolean?", is_boolean_prim},
    {"number?", is_number_prim},
    {"symbol?", is_symbol_prim},
    {"string?", is_string_prim},
    {"pair?", is_pair_prim},
    {"vector?", is_vector_prim},
    {"procedure?", is_procedure_prim},
    {"abs", abs_prim},
    {"sqrt", sqrt_prim},
    {"sin", sin_prim},
    {"cos", cos_prim},
    {"log", log_prim},
    {"max", max_prim},
    {"min", min_prim},
    {"even?", is_even},
    {"odd?", is_odd},
    {"ceil", ceil_prim},
    {"floor", floor_prim},
    {"round", round_prim},
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
    {"make-vector", make_vector},
    {"vector-set!", vector_set},
    {"vector-ref", vector_ref},
    {"vector-length", vector_length},
    {"map", map_prim},
    {"filter", filter_prim},
    {"error", error_prim}
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
