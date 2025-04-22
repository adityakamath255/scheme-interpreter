#include "types.hpp"
#include <string_view>

namespace Scheme {

class Parser {
private:
  const std::vector<std::string_view>& tokens;
  size_t index;

  Obj parse_impl(bool);

public:
  Parser(const std::vector<std::string_view>&);
  Obj parse();
};

}
