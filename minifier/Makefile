CXXFLAGS ?= -Wall -Wextra
LDFLAGS ?= -lzstd
CXX ?= g++

OBJECTS := main.o region_reader.o nbt_impl.o chunk_loader.o palette.o \
	segment_minifier.o zseg_writer.o zstd_writer.o
HEADERS := $(wildcard ../include/minifier/*)
EXECUTABLE := main
INCLUDES := -I ../include/minifier

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
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c

nbt_impl.o: nbt_impl.c
	$(CC) $(CFLAGS) $< -c

.PHONY: clean
clean:
	$(RM) $(EXECUTABLE) *.o
