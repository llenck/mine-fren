SOURCE_LIB := ../extractor-lib/libzseg_i.wasm.a

zseg_i.js: $(SOURCE_LIB)
	emcc $(SOURCE_LIB) -o $@ \
		-s EXPORTED_FUNCTIONS='["_parse_segment","_free_segment","_emalloc","_efree"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall"]'

.PHONY: clean $(SOURCE_LIB)

$(SOURCE_LIB):
	$(MAKE) -C ../extractor-lib/

clean:
	$(RM) *.wasm zseg_i.js
