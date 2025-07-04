CXX := g++
CXXFLAGS := -std=c++23 -O2

SRC := \
	types.cpp \
	environment.cpp \
	builtins/numeric.cpp \
	builtins/data.cpp \
	builtins/predicates.cpp \
	builtins/misc.cpp \
	primitives.cpp \
	expressions.cpp \
	evaluation.cpp \
	lexer.cpp \
	parser.cpp \
	memory.cpp \
	interpreter.cpp \
	main.cpp

HDR := \
	types.hpp \
	environment.hpp \
	builtins/common.hpp \
	primitives.hpp \
	expressions.hpp \
	evaluation.hpp \
	lexer.hpp \
	parser.hpp \
	memory.hpp \
	interpreter.hpp 

OBJ := $(SRC:.cpp=.o)

TARGET := scheme

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
