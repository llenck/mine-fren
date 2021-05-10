#pragma once

#include <functional>

#include "ringbuf.hpp"
#include "segment_minifier.hpp"

struct ZsegWriter {
	std::function<ssize_t(const uint8_t*, size_t)> wfn;
	std::function<bool()> cfn;

	RingBuffer<uint8_t, false> buf{2};

	bool err = false;

	ZsegWriter(
		std::function<ssize_t(const uint8_t*, size_t)> writefn,
		std::function<bool()> closefn
	);
	ZsegWriter(const ZsegWriter& other) = delete;
	ZsegWriter(ZsegWriter&& other) = delete;

	bool put_minifier(SegmentMinifier& m);

private:
	bool put_palette(uint16_t id, const std::string& name);
	bool end_palette();

	bool put_block(uint16_t block, long int dist, bool diff_is_air);

	bool full_flush();
	bool partial_flush();
};
