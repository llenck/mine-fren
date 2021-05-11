#pragma once

#include <functional>
#include <stdexcept>
#include <memory>

#include <zstd.h>

#include "ringbuf.hpp"

struct ZstdContext {
	std::shared_ptr<ZSTD_CCtx> ctx{
		ZSTD_createCCtx(),
		[] (ZSTD_CCtx* c) {
			ZSTD_freeCCtx(c);
		}
	};

	ZstdContext() : ZstdContext(9) {}
	ZstdContext(int comp_level) {
		size_t r = ZSTD_CCtx_setParameter(*this, ZSTD_c_checksumFlag, 1);
		if (ZSTD_isError(r))
			throw std::runtime_error(ZSTD_getErrorName(r));

		r = ZSTD_CCtx_setParameter(*this, ZSTD_c_compressionLevel, comp_level);
		if (ZSTD_isError(r))
			throw std::runtime_error(ZSTD_getErrorName(r));
	}

	operator ZSTD_CCtx*() {
		return ctx.get();
	}
};

struct ZstdWriter {
	std::function<ssize_t(const uint8_t*, size_t)> wfn;
	std::function<bool()> cfn;

	RingBuffer<uint8_t, false> buf{4};
	bool flushed = true;

	ZstdContext ctx;

	ZstdWriter(
		std::function<ssize_t(const uint8_t*, size_t)> writefn,
		std::function<bool()> closefn,
		ZstdContext zctx
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

