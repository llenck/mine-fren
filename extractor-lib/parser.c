#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// for ZSTD_decompressBound
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

#include "zseg-parser.h"

static int parse_palette(struct Segment* s, off_t* off, int* removed_id, int* air_id) {
	*off = 0;

	while (1) {
		const char* next = (const char*)(s->data + *off);
		if (*next == '\0') {
			*off += 3; // skip the id
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

		if (strcmp(next, "removed") == 0)
			*removed_id = *next_id;
		else if (strcmp(next, "air") == 0)
			*air_id = *next_id;

		const char* n = strdup(next);
		if (n == NULL)
			return -3;

		s->palette_names[s->palette_length - 1] = n;
		s->palette_ids[s->palette_length - 1] = *next_id;

		*off += end - next;
		*off += 3;
	}
}

static int read_varint(int64_t* ret, const uint8_t* p, int max_length) {
	int num_read = 0;
	*ret = 0;
	uint8_t read;
	do {
		if (num_read >= max_length) return -1;
		if (num_read >= 10) return -1;

		read = p[num_read];
		int val = (read & 0x7F);
		*ret |= (val << (7 * num_read));

		num_read += 1;
	} while ((read & 0x80) != 0);

	return num_read;
}

static int parse_blocks(struct Segment* s, size_t off, uint16_t removed, uint16_t air) {
	const size_t max_blocks = 128 * 128 * 256;
	size_t block_off = 0;

	int64_t next_dist;

	while (off < s->length) {
		if (block_off >= max_blocks) {
			return -1;
		}

		int varint_length = read_varint(&next_dist, s->data + off, s->length - off - 2);
		if (varint_length <= 0) {
			return -2;
		}
		else if (block_off + next_dist + 2 >= max_blocks) {
			return -3;
		}

		off += varint_length;

		uint16_t* tmp = (uint16_t*)(s->data + off);
		uint16_t next_id = *tmp & 0x7FFF;
		int diff_is_removed = *tmp & 0x8000;

		off += 2;

		uint16_t filler = diff_is_removed? removed : air;
		for (int i = 0; i < next_dist; i++) {
			s->blocks[block_off++] = filler;
		}

		s->blocks[block_off++] = next_id;
	}
	while (block_off < max_blocks) {
		// assume air, as it cannot be removed, as then the top of the world would need
		// to be saved again, with the top level of the segment being adjacent to
		// empty space
		s->blocks[block_off++] = air;
	}

	return 0;
}

void free_segment(struct Segment* s) {
	for (unsigned i = 0; i < s->palette_length; i++) {
		free((void*)s->palette_names[i]);
	}

	free((void*)s->blocks);
	free((void*)s->palette_names);
	free((void*)s->palette_ids);

	s->blocks = NULL;
	s->palette_names = NULL;
	s->palette_ids = NULL;

	if (s->allocated) {
		free((void*)s->data);
		s->allocated = 0;
		s->data = NULL;
	}
}

static uint8_t* decompress_zstd(const uint8_t* src, size_t length, size_t* dst_len) {
	size_t dec_bound = ZSTD_decompressBound(src, length);
	if (ZSTD_isError(dec_bound)) {
		return NULL;
	}

	uint8_t* ret = (uint8_t*)malloc(dec_bound);
	size_t dec_sz = ZSTD_decompress(ret, dec_bound, src, length);

	if (ZSTD_isError(dec_sz)) {
		free(ret);
		return NULL;
	}

	*dst_len = dec_sz;

	// try to free up some memory since we probably allocated too much
	uint8_t* reduced = (uint8_t*)realloc(ret, dec_sz);
	if (reduced == NULL) {
		return ret;
	}
	else {
		return reduced;
	}
}

int parse_segment(const uint8_t* data, size_t length, struct Segment* out) {
	const char _magic[] = { 0x28, 0xb5, 0x2f, 0xfd };
	const uint32_t magic = *(uint32_t*)_magic;

	if (length < 5)
		return -1;

	if (*(uint32_t*)data == magic) {
		out->allocated = 1;
		out->data = decompress_zstd(data, length, &out->length);
		if (out->data == NULL) {
			return -2;
		}
	}
	else {
		out->allocated = 0;
		out->data = data;
		out->length = length;
	}

	out->blocks = (uint16_t*)malloc(128 * 128 * 256 * sizeof(uint16_t));
	if (out->blocks == NULL) {
		return -3;
	}

	out->palette_length = 0;
	out->palette_names = NULL;
	out->palette_ids = NULL;

	int removed_id = 0, air_id = 0;

	off_t off;
	if (parse_palette(out, &off, &removed_id, &air_id) != 0) {
		free_segment(out);
		return -4;
	}

	if (parse_blocks(out, off, removed_id, air_id) != 0) {
		free_segment(out);
		return -5;
	}

	return 0;
}

#ifdef __EMSCRIPTEN__

uint64_t emalloc(size_t n) {
	return (uint64_t)malloc(n);
}
void efree(uint64_t p) {
	free((void*)p);
}

#endif
