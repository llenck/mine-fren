CXXFLAGS ?= -Wall -Wextra
LDFLAGS ?= -lzstd
CXX ?= g++

SOURCES := main.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
HEADERS :=
EXECUTABLE := main
INCLUDES := -I ../include/minifier/
MINIFY_LIB := ../minifier-lib/libminify.a

debug : CXXFLAGS += -Og -g
release : CXXFLAGS += -O3
debug : LDFLAGS +=
release : LDFLAGS += -O
debug : BUILD_TYPE := debug
release : BUILD_TYPE := release

debug: $(EXECUTABLE)
release: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(HEADERS) $(MINIFY_LIB)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(MINIFY_LIB) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c

.PHONY: clean $(MINIFY_LIB)

$(MINIFY_LIB):
	$(MAKE) -C ../minifier-lib $(BUILD_TYPE)

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
