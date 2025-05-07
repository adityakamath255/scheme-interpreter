#pragma once
#include <parsing/lexer.hpp>
#include <core/types.hpp>
#include <core/interpreter.hpp>

namespace Scheme {

class Parser {
private:
  const std::vector<Token>& tokens;
  size_t index;
  Interpreter& interp;

  bool at_end();
  const Token& advance();
  const Token& curr_token();
  const Token& next_token();
  bool match(const Token::Type);
  Obj number();
  Obj symbol();
  Obj string();
  Obj character();
  Obj parse_atom();
  Obj parse_list();
  Obj parse_vec();
  Obj parse_dotted_tail();
  Obj parse_quoted(const std::string&);

public:
  Parser(const std::vector<Token>&, Interpreter&);
  Obj parse();
};

}
