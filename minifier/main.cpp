#include <cstdio>
#include <cstdint>
#include <cerrno>

#include "ringbuf.hpp"
#include "region_reader.hpp"
#include "segment_minifier.hpp"
#include "palette.hpp"

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
	BlockIdPalette& palette;
	int fd = -1;

	RingBuffer<uint8_t, false> buf{2};

	ZsegWriter(BlockIdPalette& _palette, int segx, int segz, int dirfd=AT_FDCWD)
		: palette(_palette)
	{
		if (buf.err())
			return;

		char filename[64];
		sprintf(filename, "seg.%d.%d.zseg", segx, segz);

		fd = openat(dirfd, filename, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644);
	}
	~ZsegWriter() {
		if (fd >= 0) {
			full_flush();
			close(fd);
		}
	}

	bool full_flush() {
		errno = 0;

		while (buf.read_cap() > 0 && (errno == 0 || errno == EAGAIN)) {
			errno = 0;
			partial_flush();
		}

		return errno == 0;
	}

	void partial_flush() {
		const uint8_t* rp = buf.read_ptr();
		int rcap = buf.read_cap();

		ssize_t n = write(fd, rp, rcap);
		if (n < 0)
			return;

		buf.adv_read_ptr(n);
	}

	bool put_block(uint16_t block, long int dist, bool diff_is_air) {
		uint8_t* wr = buf.write_ptr();
		int wcap = buf.write_cap();

		if (wcap < 12) {
			close(fd);
			fd = -1;
			return false;
		}

		int written = 0;
		write_varint(dist, wr, &written);
		wr += written;

		// highest bit of block says whether this one is seperated by air from the
		// previous one or by solid hidden blocks. if the dist is 0, prefer not setting
		// the bit, to increase the frequency of blocks without the bit. tests show that
		// this is more compression-friendly than preferring to set the bit.
		if (!diff_is_air && dist != 0)
			block |= 0x8000;

		uint16_t* tmp = (uint16_t*)wr;
		*tmp = block;
		written += 2;

		buf.adv_write_ptr(written);

		// if the next put_block would be close to too much, try to flush
		// (we don't wait for the buffer to be exactly full so that we don't crash if
		// the write operation gets interrupted)
		if (wcap - written < 32)
			partial_flush();

		return true;
	}

	bool put_blocks(std::unique_ptr<chunk_data_t> blocks) {
		uint16_t dist = 0;
		uint16_t last_block = palette.removed;

		for (uint16_t b : *blocks) {
			if (b == palette.removed || b == palette.air) {
				last_block = b;
				dist++;
				continue;
			}

			if (!put_block(b, dist, last_block == palette.air))
				return false;
			dist = 0;
		}

		return true;
	}

	bool err() {
		return fd < 0;
	}
};

void output_blocks(std::unique_ptr<chunk_data_t>& blocks) {
	for (int y = 0; y < 256; y++) {
		printf("y: %d\n", y);
		for (int z = 0; z < 128; z++) {
			for (int x = 0; x < 128; x++) {
				long n = seg_xyz_to_index(x, y, z);
				uint16_t b = (*blocks)[n];

				putchar(b);
			}
			printf("\n");
		}
		printf("\n");
	}
}

int main() {
	RegionReader rd("r.-1.0.mca");

	if (rd.err())
		return 1;

//	for (int i = 0; i < 16; i++) {
//		SegmentMinifier m(rd, i % 4, i / 4);
//		auto blocks = m.minify();
//	}

	SegmentMinifier m(rd, 2, 3);
	auto blocks = m.minify();

	ZsegWriter wr(m.palette, 1, 2);
	if (wr.err())
		return 1;

	output_blocks(blocks);

	wr.put_blocks(std::move(blocks));
	
	printf("%s\n", m.palette.serialize().c_str());
}
