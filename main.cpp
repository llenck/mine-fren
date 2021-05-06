#include <cstdio>
#include <cstdint>

#include "ringbuf.hpp"
#include "region_reader.hpp"
#include "segment_minifier.hpp"

static void write_varint(int64_t value, uint8_t* target, int* bytes_written) {
	*bytes_written = 0;
	do {
		uint8_t t = (uint8_t) (value & 0x7F);
		value >>= 7;
		if (value != 0) {
			t |= 0x80;
		}
		target[*bytes_written] = t;
		*bytes_written += 1;
	} while (value != 0);
}

// TODO: compression
struct ZsegWriter {
	FILE* f = nullptr;

	RingBuffer<uint8_t, false> buf(2);

	ZsegWriter(int segx, int segz) {
		if (buf.err())
			return;

		char filename[64];
		sprintf(filename, "seg.%d.%d.zseg", segx, segz);

		FILE* f = fopen(filename, "wb");
	}
	~ZsegWriter() {
		if (f)
			fclose(f);
	}

	void partial_flush() {
		const uint8_t* rp = buf.read_ptr();
		int rcap = buf.read_cap();

		int n = fwrite(rp, 1, rcap, f);
		if (n < 0)

		buf.adv_read_ptr(fwrite(rp, 1, rcap, f));
	}

	bool put_block(uint16_t block, long int dist, bool diff_is_air) {
		uint8_t* wr = buf.write_ptr();
		int wcap = buf.write_cap();

		if (wcap < 12)
			return false;

		// highest bit of block says whether this one is seperated by air from the
		// previous one or by solid hidden blocks
		if (!diff_is_air)
			block |= 0x8000;


	}

	bool err() {
		return !f;
	}
};

int main() {
	RegionReader rd("r.0.0.mca");

	if (rd.err())
		return 1;

	int16_t last = 0;

	for (int i = 0; i < 16; i++) {
		SegmentMinifier m(rd, i % 4, i / 4);
		auto blocks = m.minify();

		for (uint16_t a : *blocks) {

		}
	}
}
