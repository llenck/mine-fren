CFLAGS ?= -Wall -Wextra

SOURCES := parser.c
OBJECTS := parser.o
HEADERS := ../include/zseg-parser.h
INCLUDES := -I ../include/
NATIVE_LIB := libzseg_i.a

debug : CFLAGS += -Og -g
release : CFLAGS += -O3

debug: $(NATIVE_LIB)
release: $(NATIVE_LIB)

$(NATIVE_LIB): $(SOURCES) $(HEADERS)
	$(CC) -c $(SOURCES) $(INCLUDES) $(LDFLAGS)
	$(RM) $(NATIVE_LIB)
	$(AR) -rcs $(NATIVE_LIB) $(OBJECTS)

.PHONY: clean
clean:
	$(RM) $(NATIVE_LIB) *.o
