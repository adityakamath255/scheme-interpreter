#pragma once
#include "../types.hpp"
#include <stdexcept>
#include <string>

namespace Scheme {

constexpr int MAX_ARGS = 1'000'000;

template<typename T>
inline void
assert_obj_type(const Obj& obj, const std::string& type) {
  if (!std::holds_alternative<T>(obj)) {
    throw std::runtime_error("incorrect type for " + stringify(obj) + ", expected " + type);
  }
}

template<typename T>
inline void
assert_vec_type(const ArgList& args, const std::string& type) {
  for (size_t i = 0; i < args.size(); i++) {
    assert_obj_type<T>(args[i], type);
  }
}

inline void
assert_callable(const Obj& obj) {
  if (!is_procedure(obj) && !is_primitive(obj)) {
    throw std::runtime_error("incorrect type for " + stringify(obj) + ", expected procedure");
  }
}

static void
assert_list(const Obj& obj) {
  if (!is_list(obj)) {
    throw std::runtime_error(stringify(obj) + " is not a proper list");
  }
}

inline void
assert_arg_count(const ArgList& args, int lb, int rb) {
  if (!(lb <= args.size() && args.size() <= rb)) {
    if (rb == MAX_ARGS)
      throw std::runtime_error("incorrect number of arguments: expected at least " + std::to_string(lb));
    else if (lb == rb)
      throw std::runtime_error("incorrect number of arguments: expected " + std::to_string(lb));
    else
      throw std::runtime_error("incorrect number of arguments: expected between " + std::to_string(lb) + " and " + std::to_string(rb));
  }
}

inline void assert_numbers(const ArgList& args, int lb, int rb) {
  assert_arg_count(args, lb, rb);
  for (const auto& arg : args) {
    if (!is_number(arg))
      throw std::runtime_error("expected number, got " + stringify(arg));
  }
}

inline double get_single_number(const ArgList& args) {
  assert_numbers(args, 1, 1);
  return as_number(args[0]);
}

}
