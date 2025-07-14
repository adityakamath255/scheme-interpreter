#include <builtins/common.hpp>
#include <builtins/installer.hpp>
#include <interpreter/evaluation.hpp>

namespace Scheme {

void 
BuiltinInstaller::install_data_functions() {
  install("car", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    assert_obj_type<Cons*>(args[0], "pair");
    return as_pair(args[0])->car;
  });

  install("cdr", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    assert_obj_type<Cons*>(args[0], "pair");
    return as_pair(args[0])->cdr;
  });

  install("not", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_false(args[0]);
  });

  install("cons", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    return interp.spawn<Cons>(args[0], args[1]);
  });

  install("list", [](const ArgList& args, Interpreter& interp) {
    Obj ret = Null {};
    for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
      ret = interp.spawn<Cons>(*curr, ret);
    }
    return ret;
  });

  install("set-car!", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_obj_type<Cons*>(args[0], "list");
    as_pair(args[0])->car = args[1];
    return Void {};
  });

  install("set-cdr!", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_obj_type<Cons*>(args[0], "list");
    as_pair(args[0])->cdr = args[1];
    return Void {};
  });

  install("length", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return (double) list_length(args[0]);
  });

  install("list-ref", [](const ArgList& args, Interpreter& interp) {
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
  });

  install("symbol->string", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    assert_obj_type<Symbol>(args[0], "symbol");
    return interp.spawn<String>(as_symbol(args[0]).get_name());
  });

  install("string->symbol", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    assert_obj_type<String*>(args[0], "string");
    return interp.intern_symbol(as_string(args[0])->data);
  });

  install("string-append", [](const ArgList& args, Interpreter& interp) {
    assert_vec_type<String*>(args, "string");
    std::stringstream ret {};
    for (auto obj : args) {
      ret << as_string(obj)->data;
    }
    return interp.spawn<String>(ret.str());
  });

  install("make-vector", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 2);
    assert_obj_type<double>(args[0], "number");
    const size_t sz = as_number(args[0]);
    if (sz < 0) {
      throw std::runtime_error("vector size cannot be negative");
    }
    if (args.size() == 2) {
      return interp.spawn<Vector>(
        std::vector<Obj>(sz, args[1])
      );
    }
    else {
      return interp.spawn<Vector>(
        std::vector<Obj>(sz, Obj {(double) 0})
      );
    }
  });

  install("vector-set!", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 3, 3);
    assert_obj_type<Vector*>(args[0], "vector");
    assert_obj_type<double>(args[1], "number");
    const size_t index = as_number(args[1]);
    if (index < 0) {
      throw std::runtime_error("vector index cannot be negative");
    }
    std::vector<Obj>& data = as_vector(args[0])->data;
    if (index >= data.size()) {
      throw std::runtime_error("vector index out of range");
    }
    data[index] = args[2];
    return Void {};
  });

  install("vector-ref", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_obj_type<Vector*>(args[0], "vector");
    assert_obj_type<double>(args[1], "number");
    const size_t index = as_number(args[1]);
    if (index < 0) {
      throw std::runtime_error("vector index cannot be negative");
    }
    std::vector<Obj>& data = as_vector(args[0])->data;
    if (index >= data.size()) {
      throw std::runtime_error("vector index out of range");
    }
    return data[index]; 
  });

  install("vector-length", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    assert_obj_type<Vector*>(args[0], "vector");
    return (double) as_vector(args[0])->data.size();
  });

}

}
