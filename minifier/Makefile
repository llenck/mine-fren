CXXFLAGS ?= -Wall -Wextra
LDFLAGS ?=
CXX ?= g++

OBJECTS := main.o region_reader.o nbt-impl.o chunk_loader.o palette.o segment_minifier.o zseg-writer.o
HEADERS := region_reader.hpp nbt.hpp chunk_loader.hpp palette.hpp segment_minifier.hpp ringbuf.hpp zseg-writer.hpp
EXECUTABLE := main

debug : CXXFLAGS += -Og -g
release : CXXFLAGS += -O3
debug : CFLAGS += -Og -g
release : CFLAGS += -O3
debug : LDFLAGS +=
release : LDFLAGS += -O

debug: $(EXECUTABLE)
release: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< -c

nbt-impl.o: nbt-impl.c
	$(CC) $(CFLAGS) $< -c

.PHONY: clean
clean:
	$(RM) $(EXECUTABLE) *.o
