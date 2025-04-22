#include "types.hpp"

namespace Scheme {

class Lexer {
private:
  const std::string_view input;
  size_t start;
  size_t curr;
  std::vector<std::string_view> tokens;

  void insert(const bool);
  void skip_whitespace();

public:
  Lexer(const std::string_view);
  std::vector<std::string_view> tokenize();
};

}
