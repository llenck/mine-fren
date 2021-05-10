#pragma once

#include "ringbuf.hpp"
#include "segment_minifier.hpp"

struct ZsegWriter {
	int fd = -1;

	RingBuffer<uint8_t, false> buf{2};

	ZsegWriter(int segx, int segz, int dirfd=AT_FDCWD);
	ZsegWriter(const ZsegWriter& other) = delete;
	ZsegWriter(ZsegWriter&& other) = delete;

	~ZsegWriter() {
		if (fd >= 0) {
			full_flush();
			close(fd);
		}
	}

	bool err() {
		return fd < 0;
	}

	bool put_minifier(SegmentMinifier& m);

private:
	bool put_palette(uint16_t id, const std::string& name);
	bool end_palette();

	bool put_block(uint16_t block, long int dist, bool diff_is_air);

	bool full_flush();
	void partial_flush();
};
