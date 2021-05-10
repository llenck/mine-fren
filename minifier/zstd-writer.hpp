#pragma once

#include <functional>
#include <stdexcept>
#include <memory>

#include <zstd.h>

#include "ringbuf.hpp"

struct ZstdWriter {
	std::function<ssize_t(const uint8_t*, size_t)> wfn;
	std::function<bool()> cfn;

	RingBuffer<uint8_t, false> buf{4};
	bool flushed = true;

	std::unique_ptr<ZSTD_CCtx, std::function<void(ZSTD_CCtx*)>> ctx{nullptr,
			[] (ZSTD_CCtx* c) { ZSTD_freeCCtx(c); }};

	ZstdWriter(
		std::function<ssize_t(const uint8_t*, size_t)> writefn,
		std::function<bool()> closefn,
		int comp_level=15
	);
	ZstdWriter(const ZstdWriter& other) = delete;
	ZstdWriter(ZstdWriter&& other) = delete;

	~ZstdWriter();

	// arguments and return value are similar to what write(2) does. throws if it
	// couldn't process the whole thing and end is true
	ssize_t write(const uint8_t* data, size_t n, bool end=false);

	void full_flush();
	void partial_flush();
};

