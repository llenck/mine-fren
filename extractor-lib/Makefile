CXXFLAGS ?= -Wall -Wextra
LDFLAGS ?=
CXX ?= g++

OBJECTS := main.o
HEADERS := view.hpp
EXECUTABLE := main
INCLUDES := -I ../common

debug : CXXFLAGS += -Og -g
release : CXXFLAGS += -O3
debug : LDFLAGS +=
release : LDFLAGS += -O

debug: $(EXECUTABLE)
release: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c

.PHONY: clean
clean:
	$(RM) $(EXECUTABLE) *.o
