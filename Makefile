CXX := g++
CXXFLAGS := -std=c++23 -O2

SRC := \
  main.cpp \
  common.cpp \
  stringify.cpp \
  evaluation.cpp \
  expressions.cpp \
  parsing.cpp \
  primitives.cpp \
  tco.cpp

HDR := \
  common.hpp \
  display.hpp \
  expressions.hpp \
  parsing.hpp \
  primitives.hpp

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
