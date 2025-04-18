#include "types.hpp"

namespace Scheme {

class Lexer {
private:
  const string& input;
  size_t start;
  size_t curr;
  vector<string> tokens;

  void insert(const bool);

public:
  Lexer(const string&);
  vector<string> tokenize();
};

}
