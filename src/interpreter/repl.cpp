#include <optional>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <repl.hpp>

std::optional<size_t>
BracketChecker::read() {
  pos = 0;
  while (!at_end() && state != State::Finished) {
    process();
  }
  if (state == State::Finished) {
    return pos;
  }
  else {
    return std::nullopt;
  }
}

bool 
BracketChecker::at_end() const {
  return pos >= line.size();
}

void 
BracketChecker::reset() {
  set_state(State::Normal);
  brackets.clear();
  block_comment_depth = 0;
}

void 
BracketChecker::set_state(const State next_state) {
  state = next_state;
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
  switch (state) {
    case State::Normal:
      process_normal();
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

    case State::SeenHashInNormal:
      process_seen_hash_from_normal();
      break;

    case State::SeenHashInBlockComment:
      throw std::runtime_error("invalid state");

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
BracketChecker::process_normal() {
  switch (advance()) {
    case '"':
      set_state(State::String);
      break;

    case ';':
      set_state(State::LineComment);
      break;

    case '#':
      set_state(State::SeenHashInNormal);
      break;

    case '|':
      throw std::runtime_error("unexpected '|'");

    case '(':
    case '[':
      push_bracket();
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
      set_state(State::Normal);
      break;

  }
}

void
BracketChecker::process_escaped() {
  advance();
  set_state(State::String);
}

void 
BracketChecker::process_line_comment() {
  skip_line();
  set_state(State::Normal);
}

void 
BracketChecker::process_seen_hash_from_normal() {
  if (peek() == '|') {
    set_state(State::BlockComment);
    block_comment_depth = 1;
  }
  else {
    set_state(State::Normal);
    retract();
  }
}

void 
BracketChecker::process_seen_hash_from_block_comment() {
  if (advance() == '|') {
    block_comment_depth++;
  }
  else {
    set_state(State::BlockComment);
  }
}

void
BracketChecker::process_seen_pipe() {
  if (advance() == '#' && block_comment_depth > 0) {
    block_comment_depth--;
    if (block_comment_depth == 0) {
      set_state(State::Normal);
    }
    else {
      set_state(State::BlockComment);
    }
  }
  else {
    set_state(State::BlockComment);
  }
}

void
BracketChecker::process_block_comment() {
  switch (advance()) {
    case '\\':
      set_state(State::Escaped);
      break;

    case '#':
      set_state(State::SeenHashInBlockComment);
      break;

    case '|':
      set_state(State::SeenPipe);
      break;
  }
}

void 
BracketChecker::process_finished() {
  throw std::runtime_error("BracketChecker object needs to be reset");
}
