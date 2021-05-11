CFLAGS ?= -Wall -Wextra

SOURCES := parser.c
OBJECTS := parser.o
WASM_OBJECTS := parser.wasm.o
HEADERS := ../include/zseg-parser.h
INCLUDES := -I ../include/
NATIVE_LIB := libzseg_i.a
WASM_LIB := libzseg_i.wasm.a

debug : CFLAGS += -Og -g
release : CFLAGS += -O3

debug: $(NATIVE_LIB) #$(WASM_LIB)
release: $(NATIVE_LIB) #$(WASM_LIB)

$(NATIVE_LIB): $(SOURCES) $(HEADERS)
	$(CC) -c $(SOURCES) $(INCLUDES) $(CFLAGS)
	$(RM) $(NATIVE_LIB)
	$(AR) -rcs $(NATIVE_LIB) $(OBJECTS)

$(WASM_LIB): $(WASM_OBJECTS)
	$(RM) $(WASM_LIB)
	emar -rcs $(WASM_LIB) $(WASM_OBJECTS)

%.wasm.o: $(SOURCES) $(HEADERS)
	emcc -c $(INCLUDES) $< -o $@

.PHONY: clean
clean:
	$(RM) $(NATIVE_LIB) *.o
