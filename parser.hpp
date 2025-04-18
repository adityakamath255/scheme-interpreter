#include "types.hpp"

namespace Scheme {

class Parser {
private:
  const vector<string>& tokens;
  size_t index;

  Obj* parse_impl(bool);

public:
  Parser(const vector<string>&);
  Obj parse();
};

}
