#ifndef _ZSEG_PARSER_H_INCLUDED
#define _ZSEG_PARSER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

struct Segment {
	uint16_t* blocks;

	const char** palette_names;
	uint16_t* palette_ids;
	unsigned palette_length;

	// private
	char allocated;
	const uint8_t* data;
	size_t length;
};

int parse_segment(const uint8_t* data, size_t length, struct Segment* out);
void free_segment(struct Segment* s);

#ifdef __EMSCRIPTEN__
uint64_t emalloc(size_t n);
void efree(uint64_t p);
#endif

#ifdef __cplusplus
}
#endif

#endif
