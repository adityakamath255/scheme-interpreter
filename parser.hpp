#pragma once
#include "types.hpp"
#include "interpreter.hpp"
#include <string_view>

namespace Scheme {

class Parser {
private:
  const std::vector<std::string_view>& tokens;
  size_t index;
  Interpreter& interp;

  std::string_view next_token();
  bool match(const std::string_view);
  Obj make_num_obj(const std::string_view);
  Obj make_sym_obj(const std::string_view);
  Obj make_bool_obj(const std::string_view);
  Obj make_str_obj(const std::string_view);
  Obj from_str(const std::string_view);
  Obj parse_atom();
  Obj parse_list();
  Obj parse_vec();
  Obj parse_dotted_tail();
  Obj parse_quoted(const std::string_view);

public:
  Parser(const std::vector<std::string_view>&, Interpreter&);
  Obj parse();
};

}
