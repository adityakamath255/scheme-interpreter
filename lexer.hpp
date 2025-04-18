#include "types.hpp"

namespace Scheme {

class Lexer {
private:
  const std::string_view input;
  size_t start;
  size_t curr;
  vector<std::string_view> tokens;

  void insert(const bool);

public:
  Lexer(const std::string_view);
  vector<std::string_view> tokenize();
};

}
