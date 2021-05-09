CXXFLAGS ?= -Wall -Wextra -Og -g
LDFLAGS ?= ../extractor-lib/libzseg_i.a
CXX ?= g++

OBJECTS := main.o
HEADERS := view.hpp
CXXFLAGS += -I ../include

main: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.PHONY: clean ../extractor-lib/libzseg_i.a

../extractor-lib/libzseg_i.a:
	$(MAKE) -C ../extractor-lib/

clean:
	$(RM) main *.o
