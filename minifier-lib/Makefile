CXXFLAGS ?= -Wall -Wextra
CXX ?= g++

SOURCES := region_reader.cpp nbt_impl.cpp chunk_loader.cpp palette.cpp \
	segment_minifier.cpp zseg_writer.cpp zstd_writer.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES)) nbt_impl.o
HEADERS := $(wildcard ../include/minifier/*)
LIBRARY := libminify.a
INCLUDES := -I ../include/minifier

debug : CXXFLAGS += -Og -g
release : CXXFLAGS += -O3
debug : CFLAGS += -Og -g
release : CFLAGS += -O3
debug : LDFLAGS +=
release : LDFLAGS += -O

debug: $(LIBRARY)
release: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(RM) $@
	$(AR) -rcs $@ $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c

nbt_impl.o: nbt_impl.c
	$(CC) $(CFLAGS) $< -c

.PHONY: clean
clean:
	$(RM) $(LIBRARY) $(OBJECTS)
