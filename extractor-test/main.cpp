#include <cstdio>

#include "view.hpp"
#include "zseg-parser.h"

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

	uint16_t last_id = s.blocks[0];
	long last_id_cnt = 0;

	for (long i = 0; i < 128 * 128 * 256; i++) {
		uint16_t cur = s.blocks[i];

		if (last_id == cur) {
			last_id_cnt++;
		}
		else {
			printf("%ld * %hu\n", last_id_cnt, last_id);

			last_id_cnt = 1;
			last_id = cur;
		}
	}

	free_segment(&s);

	puts("Done :)");
}
