CFLAGS ?= -Wall -Wextra
LDFLAGS ?=
CXX ?= g++

OBJECTS := main.o
HEADERS :=
EXECUTABLE := main

debug : CFLAGS += -Og -g -pg
release : CFLAGS += -O3
debug : LDFLAGS +=
release : LDFLAGS += -O -s

debug: $(EXECUTABLE)
release: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $< -c

.PHONY: clean
clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
