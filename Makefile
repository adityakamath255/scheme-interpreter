CXX := g++
CXXFLAGS := -std=c++23 -O2

SRC := \
	types.cpp \
	environment.cpp \
	stringify.cpp \
	primitives.cpp \
	expressions.cpp \
	evaluation.cpp \
	tco.cpp \
	lexer.cpp \
	parser.cpp \
	interpreter.cpp \
	main.cpp

HDR := \
	types.hpp \
	environment.hpp \
	stringify.hpp \
	primitives.hpp \
	expressions.hpp \
	evaluation.hpp \
	tco.hpp \
	lexer.hpp \
	interpreter.hpp \
	parser.hpp

OBJ := $(SRC:.cpp=.o)

TARGET := scheme

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)
