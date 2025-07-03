#include <optional>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <repl.hpp>

BracketChecker::BracketChecker():
  line {},
  state_stk {State::Initial},
  bracket_stk {},
  block_comment_depth {0},
  pos {0}
{}

std::optional<size_t>
BracketChecker::read(const std::string_view line) {
  this->line = line;
  pos = 0;
  while (!at_end() && curr_state() != State::Finished) {
    process();
  }
  if (curr_state() == State::Finished) {
    return pos;
  }
  else {
    return std::nullopt;
  }
}

BracketChecker::State
BracketChecker::curr_state() {
  if (!state_stk.empty()) {
    return state_stk.back();
  }
  else {
    return State::Finished;
  }
}

void
BracketChecker::push_state(const State state) {
  state_stk.push_back(state);
}

void 
BracketChecker::pop_state() {
  if (!state_stk.empty()) {
    state_stk.pop_back();
  }
}

void 
BracketChecker::set_state(const State next_state) {
  pop_state();
  push_state(next_state);
}

bool 
BracketChecker::at_end() const {
  return pos >= line.size();
}

static bool 
matching_brackets(const char opening, const char closing) {
  return 
    (opening == '(' && closing == ')') ||
    (opening == '[' && closing == ']');
}

void
BracketChecker::push_bracket(const char b) {
  bracket_stk.push_back(b);
  if (curr_state() != State::Expression) {
    push_state(State::Expression);
  }
}

void
BracketChecker::pop_bracket(const char closing) {
  const char opening = bracket_stk.back();
  if (matching_brackets(opening, closing)) {
    bracket_stk.pop_back();
    if (bracket_stk.empty()) {
      set_state(State::Finished);
    }
  }
  else {
    throw std::runtime_error("brackets do not match");
  }
}

char 
BracketChecker::advance() {
  if (pos >= line.size()) {
    return EOF;
  }
  else {
    return line[pos++];
  }
}

void
BracketChecker::retract() {
  pos--;
}

void 
BracketChecker::skip_line() {
  pos = line.size();
}

void
BracketChecker::process() {
  switch (curr_state()) {
    case State::Initial:
      process_initial();
      break;

    case State::Term:
      process_term();
      break;

    case State::Expression:
      process_expression();
      break;

    case State::String:
      process_string();
      break;

    case State::Escaped:
      process_escaped();
      break;

    case State::LineComment:
      process_line_comment();
      break;

    case State::SeenHash:
      process_seen_hash();
      break;

    case State::SeenPipe:
      process_seen_pipe();
      break;

    case State::BlockComment:
      process_block_comment();
      break;

    case State::Finished:
      process_finished();
      break;
  }
}

void
BracketChecker::process_initial() {
  switch (advance()) {
    case ' ':
    case '\r':
    case '\t':
    case '\n':
      break;

    case '(':
    case '[':
      set_state(State::Expression);
      retract();
      break;

    case '"':
      set_state(State::String);
      break;

    case '#':
      set_state(State::SeenHash);

    default:
      set_state(State::Term);
      retract();
      break;
  }
}

void 
BracketChecker::process_term() {
  switch (advance()) {
    case ' ':
    case '\r':
    case '\t':
    case '\n':
    case ';':
    case '"':
    case '(':
    case '[':
      set_state(State::Finished);
      break;

    case ')':
    case ']':
      throw std::runtime_error("unmatched closing bracket");

    case '"':
      push_state(State::String);
  }
}

void 
BracketChecker::process_expression() {
  switch (const char c = advance()) {
    case '"':
      push_state(State::String);
      break;

    case ';':
      skip_line();
      break;

    case '#':
      push_state(State::SeenHash);
      break;

    case '|':
      throw std::runtime_error("unexpected '|'");

    case '(':
    case '[':
      push_bracket(c);
      break;

    case ')':
    case ']':
      pop_bracket();
      break;

    case EOF:
      set_state(State::Finished);
  }
}

void 
BracketChecker::process_string() {
  switch (advance()) {
    case '\\':
      set_state(State::Escaped);
      break;

    case '"':
      pop_state(); 
      break;

  }
}

void
BracketChecker::process_escaped() {
  advance();
  set_state(State::String);
}

void 
BracketChecker::process_seen_hash() {
  pop_state();
  if (advance() == '|') {
    if (curr_state() != ) {
      push_state(State::BlockComment);
    }
    block_comment_depth++;
  }
  else {
    retract();
  }
}

void
BracketChecker::process_seen_pipe() {
  pop_state();
  if (advance() == '#') {
    block_comment_depth--;
    if (block_comment_depth == 0) {
      set_state(State::Expression);
    }
    else {
      set_state(State::BlockComment);
    }
  }
  else {
    retract();
  }
}

void
BracketChecker::process_block_comment() {
  switch (advance()) {
    case '#':
      push_state(State::SeenHash);
      break;

    case '|':
      push_state(State::SeenPipe);
      break;
  }
}

void 
BracketChecker::process_finished() {
  throw std::runtime_error("BracketChecker object needs to be reset");
}
}
