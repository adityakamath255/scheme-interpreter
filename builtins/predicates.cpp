#include "../primitives.hpp"
#include "common.hpp"

namespace Scheme {

void
PrimitivePutter::put_predicates() {
  put("null?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_null(args[0]);
  });

  put("boolean?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_bool(args[0]);
  });

  put("number?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_number(args[0]);
  });

  put("pair?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_pair(args[0]);
  });

  put("vector?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_vector(args[0]);
  });

  put("symbol?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_symbol(args[0]);
  });

  put("string?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_string(args[0]);
  });

  put("procedure?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_procedure(args[0]) || is_primitive(args[0]);
  });

  put("list?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_list(args[0]);
  });
}

}

