#include "types.hpp"

namespace Scheme {

class Lexer {
private:
  const string& input;
  string curr_tok;
  vector<string> tokens;

  void insert_and_clear();

public:
  Lexer(const string&);
  vector<string> tokenize();
};

}
