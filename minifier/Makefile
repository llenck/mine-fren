CXXFLAGS ?= -Wall -Wextra
LIBS ?= -lzstd
CXX ?= g++

SOURCES := main.cpp
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
HEADERS :=
EXECUTABLE := mc-minify
INCLUDES := -I ../include
MINIFY_LIB := ../minifier-lib/libminify.a

INSTALL_PATH ?= /usr/bin/$(EXECUTABLE)

debug : CXXFLAGS += -Og -g
release : CXXFLAGS += -O3
debug : LDFLAGS +=
release : LDFLAGS += -O
debug : BUILD_TYPE := debug
release : BUILD_TYPE := release

debug: $(EXECUTABLE)
release: $(EXECUTABLE)

install : BUILD_TYPE := release
install: $(EXECUTABLE)
	install $(EXECUTABLE) $(INSTALL_PATH) -m755

uninstall:
	rm $(INSTALL_PATH)

$(EXECUTABLE): $(OBJECTS) $(HEADERS) $(MINIFY_LIB)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) $(MINIFY_LIB) -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -c

.PHONY: clean $(MINIFY_LIB)

$(MINIFY_LIB):
	$(MAKE) -C ../minifier-lib $(BUILD_TYPE)

clean:
	$(RM) $(EXECUTABLE) $(OBJECTS)
