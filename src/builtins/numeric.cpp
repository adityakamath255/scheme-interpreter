#include <builtins/common.hpp>
#include <builtins/installer.hpp>
#include <cmath>

namespace Scheme {

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

void 
BuiltinInstaller::install_numeric_functions() {
  install("+", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 0, MAX_ARGS);
    double ret = 0.0;
    for (const auto& arg : args) {
      ret += as_number(arg);
    }
    return ret;
  });
  install("-", [](const ArgList& args, Interpreter& interp) {
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
  });
  install("*", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 0, MAX_ARGS);
    double ret = 1.0;
    for (const auto& arg : args) {
      ret *= as_number(arg);
    }
    return ret;
  });
  install("/", [](const ArgList& args, Interpreter& interp) {
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
  });
  install("<", [](const ArgList& args, Interpreter& interp) {
    return check_comp(args, std::less<double>());
  });
  install(">", [](const ArgList& args, Interpreter& interp) {
    return check_comp(args, std::greater<double>());
  });
  install("=", [](const ArgList& args, Interpreter& interp) {
    return check_comp(args, std::equal_to<double>());
  });
  install("<=", [](const ArgList& args, Interpreter& interp) {
    return check_comp(args, std::less_equal<double>());
  });
  install(">=", [](const ArgList& args, Interpreter& interp) {
    return check_comp(args, std::greater_equal<double>());
  });
  install("abs", [](const ArgList& args, Interpreter& interp) {
    return std::abs(get_single_number(args));
  });
  install("sqrt", [](const ArgList& args, Interpreter& interp) {
    return std::sqrt(get_single_number(args));
  });
  install("sin", [](const ArgList& args, Interpreter& interp) {
    return std::sin(get_single_number(args));
  });
  install("cos", [](const ArgList& args, Interpreter& interp) {
    return std::cos(get_single_number(args));
  });
  install("log", [](const ArgList& args, Interpreter& interp) {
    return std::log(get_single_number(args));
  });
  install("max", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 1, MAX_ARGS);
    double ret = -INFINITY;
    for (const auto& arg : args)
      ret = std::max(ret, as_number(arg));
    return ret;
  });
  install("min", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 1, MAX_ARGS);
    double ret = INFINITY;
    for (const auto& arg : args)
      ret = std::min(ret, as_number(arg));
    return ret;
  });
  install("even?", [](const ArgList& args, Interpreter& interp) {
    return (bool) !(1 & static_cast<int>(get_single_number(args)));
  });
  install("odd?", [](const ArgList& args, Interpreter& interp) {
    return (bool) (1 & static_cast<int>(get_single_number(args)));
  });
  install("ceil", [](const ArgList& args, Interpreter& interp) {
    return std::ceil(get_single_number(args));
  });
  install("floor", [](const ArgList& args, Interpreter& interp) {
    return std::floor(get_single_number(args));
  });
  install("round", [](const ArgList& args, Interpreter& interp) {
    return std::round(get_single_number(args));
  });
  install("expt", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 2, 2);
    return std::pow(as_number(args[0]), as_number(args[1]));
  });
  install("quotient", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 2, 2);
    return static_cast<double>(
      static_cast<int>(
        as_number(args[0]) / 
        as_number(args[1])));
  });
  install("remainder", [](const ArgList& args, Interpreter& interp) {
    assert_numbers(args, 2, 2);
    return fmod(as_number(args[0]), as_number(args[1]));
  });
}

}
