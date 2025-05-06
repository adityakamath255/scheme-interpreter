#include <builtins/common.hpp>
#include <builtins/installer.hpp>

namespace Scheme {

void
BuiltinInstaller::install_predicates() {
  install("null?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_null(args[0]);
  });

  install("boolean?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_bool(args[0]);
  });

  install("number?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_number(args[0]);
  });

  install("pair?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_pair(args[0]);
  });

  install("vector?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_vector(args[0]);
  });

  install("symbol?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_symbol(args[0]);
  });

  install("string?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_string(args[0]);
  });

  install("procedure?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_procedure(args[0]) || is_primitive(args[0]);
  });

  install("list?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_list(args[0]);
  });
}

}

