#include "../primitives.hpp"
#include "../evaluation.hpp"
#include "common.hpp"

namespace Scheme {

static Obj
append_rec(const Obj& list1, const Obj& list2, Interpreter& interp) {
  if (is_pair(list1)) {
    return interp.spawn<Cons>(
      as_pair(list1)->car,
      append_rec(as_pair(list1)->cdr, list2, interp)
    );
  }
  else {
    return list2;
  }
}

static Obj
map_rec(Obj& fn, Obj& obj, Interpreter& interp) {
  if (!is_pair(obj)) {
    return obj;
  }
  else {
    auto ls = as_pair(obj);
    return interp.spawn<Cons>(
      as_obj(apply(fn, ArgList({ls->car}), interp)), 
      map_rec(fn, ls->cdr, interp)
    );
  }
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
      return interp.spawn<Cons>(ls->car, rest);
    }
    else {
      return rest;
    }
  }
}

void 
PrimitivePutter::put_data_functions() {
  put("eq?", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    return args[0] == args[1];
  });

  // put("equal?", is_equal);

  put("not", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    return is_false(args[0]);
  });

  put("cons", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    return interp.spawn<Cons>(args[0], args[1]);
  });

  put("list", [](const ArgList& args, Interpreter& interp) {
    Obj ret = Null {};
    for (auto curr = args.rbegin(); curr != args.rend(); curr++) {
      ret = interp.spawn<Cons>(*curr, ret);
    }
    return ret;
  });

  put("set-car!", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_obj_type<Cons*>(args[0], "list");
    as_pair(args[0])->car = args[1];
    return Void {};
  });

  put("set-cdr!", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_obj_type<Cons*>(args[0], "list");
    as_pair(args[0])->cdr = args[1];
    return Void {};
  });

  put("length", [](const ArgList& args, Interpreter& interp) {
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
  });

  put("list-ref", [](const ArgList& args, Interpreter& interp) {
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

  put("append", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, MAX_ARGS);
    assert_vec_type<Cons*>(args, "list");
    Obj ret = Null {};
    for (size_t i = args.size(); i-- > 0;) {
      ret = append_rec(args[i], ret, interp);
    }
    return ret;
  });

  put("make-vector", [](const ArgList& args, Interpreter& interp) {
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

  put("vector-set!", [](const ArgList& args, Interpreter& interp) {
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

  put("vector-ref", [](const ArgList& args, Interpreter& interp) {
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

  put("vector-length", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 1, 1);
    assert_obj_type<Vector*>(args[0], "vector");
    return (double) as_vector(args[0])->data.size();
  });

  put("map", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_callable(args[0]);
    assert_obj_type<Cons*>(args[1], "list");
    auto fn = args[0];
    auto ls = args[1];
    auto ret = map_rec(fn, ls, interp);
    return ret;
  });

  put("filter", [](const ArgList& args, Interpreter& interp) {
    assert_arg_count(args, 2, 2);
    assert_callable(args[0]);
    assert_obj_type<Cons*>(args[1], "list");
    auto fn = args[0];
    auto ls = args[1];
    return filter_rec(fn, ls, interp);
  });
}

}
