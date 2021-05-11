#include "zstd-writer.hpp"

ZstdWriter::ZstdWriter(
	std::function<ssize_t(const uint8_t*, size_t)> writefn,
	std::function<bool()> closefn,
	ZstdContext zctx
) : wfn(writefn), cfn(closefn), ctx(zctx)
{
	if (buf.err())
		throw std::runtime_error("Couldn't create ring buffer");
}

ZstdWriter::~ZstdWriter() {
	if (!flushed) {
		full_flush();
	}
}

ssize_t ZstdWriter::write(const uint8_t* data, size_t n, bool end) {
	ZSTD_EndDirective mode = end? ZSTD_e_end : ZSTD_e_continue;
	ZSTD_inBuffer in = { data, n, 0 };

	uint8_t* wr = buf.write_ptr();
	unsigned wcap = buf.write_cap();

	if (wcap < n / 2) {
		partial_flush();
		wr = buf.write_ptr(); // should be the same, but idk
		wcap = buf.write_cap();
	}

	ZSTD_outBuffer out = { wr, wcap, 0 };

	size_t r = ZSTD_compressStream2(ctx, &out, &in, mode);
	if (ZSTD_isError(r))
		throw std::runtime_error(ZSTD_getErrorName(r));

	buf.adv_write_ptr(out.pos);

	if (end && in.pos != in.size)
		throw std::runtime_error("Couldn't finalize zstd stream");

	flushed = false;
	return in.pos;
}

void ZstdWriter::full_flush() {
	while (buf.read_cap() > 0)
		partial_flush();
}

void ZstdWriter::partial_flush() {
	const uint8_t* rp = buf.read_ptr();
	int rcap = buf.read_cap();

	ssize_t n = wfn(rp, rcap);
	if (n < 0)
		throw std::runtime_error("ZstdWriter: Write callback error");

	if (n == rcap)
		flushed = true;

	buf.adv_read_ptr(n);
}
