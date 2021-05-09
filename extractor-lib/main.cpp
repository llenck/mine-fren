#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "view.hpp"

struct Segment {
	const uint8_t* data;
	size_t length;

	uint16_t* blocks;

	unsigned palette_length;
	const char** palette_names;
	uint16_t* palette_ids;
};

static int parse_palette(struct Segment* s, off_t* off) {
	*off = 0;

	while (1) {
		const char* next = (const char*)(s->data + *off);
		if (*next == '\0') {
			*off += 2; // skip the id
			return 0;
		}

		// reallocate things
		s->palette_length++;
		const char** tmp = (const char**)realloc(s->palette_names,
				s->palette_length * sizeof(const char*));

		if (tmp == NULL) {
			return -1;
		}
		s->palette_names = tmp;

		uint16_t* tmp2 = (uint16_t*)realloc(s->palette_ids,
				s->palette_length * sizeof(uint16_t));

		if (tmp2 == NULL) {
			return -2;
		}
		s->palette_ids = tmp2;

		// find end of buffer
		const char* end = (const char*)memchr(next, '\0', s->length - *off);
		if (end == NULL || (end + 3) - (char*)s->data >= (ssize_t)s->length) {
			return -3;
		}
		const uint16_t* next_id = (const uint16_t*)(end + 1);

		s->palette_names[s->palette_length - 1] = next;
		s->palette_ids[s->palette_length - 1] = *next_id;

		*off += end - next;
		*off += 3;
	}
}

static int parse_blocks(struct Segment* s, off_t off) {
	ssize_t blocks = 128 * 128 * 256;
	return 0;
}

void free_segment(struct Segment* s) {
	free((void*)s->blocks);
	free((void*)s->palette_names);
	free((void*)s->palette_ids);

	s->blocks = NULL;
	s->palette_names = NULL;
	s->palette_ids = NULL;
}

// data must outlive the Segment struct
int parse_segment(const uint8_t* data, size_t length, struct Segment* out) {
	out->data = data;
	out->length = length;

	out->blocks = (uint16_t*)malloc(128 * 128 * 256 * sizeof(uint16_t));
	if (out->blocks == NULL) {
		return -1;
	}

	out->palette_length = 0;
	out->palette_names = NULL;
	out->palette_ids = NULL;

	off_t off;
	if (parse_palette(out, &off) != 0) {
		free_segment(out);
		return -2;
	}

	if (parse_blocks(out, off) != 0) {
		free_segment(out);
		return -3;
	}

	return 0;
}

int main() {
	FileView f("seg.1.2.zseg");
	if (f.err()) {
		fputs("Error opening segz\n", stderr);
		return 1;
	}

	struct Segment s;
	if (parse_segment(f.map, f.sz, &s) != 0) {
		fputs("Error parsing segz\n", stderr);
		return 1;
	}

	for (unsigned i = 0; i < s.palette_length; i++) {
		printf("%s -> %d\n", s.palette_names[i], s.palette_ids[i]);
	}

	free_segment(&s);

	puts("Done :)");
}
