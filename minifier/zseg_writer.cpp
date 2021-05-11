#include "zseg_writer.hpp"

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

ZsegWriter::ZsegWriter(
		std::function<ssize_t(const uint8_t*, size_t, bool)> writefn,
		std::function<bool()> closefn)
	: wfn(std::move(writefn)), cfn(std::move(closefn))
{
	if (buf.err())
		err = true;
}

bool ZsegWriter::put_minifier(SegmentMinifier& m) {
	std::unique_ptr<chunk_data_t> blocks = m.minify();

	BlockIdPalette& p = m.palette;

	for (const std::string& k : p.requested) {
		if (!put_palette(p.p[k], k))
			return false;
	}

	if (!end_palette())
		return false;

	uint16_t dist = 0;
	uint16_t last_block = p.removed;
	for (uint16_t b : *blocks) {
		if (b == p.removed || b == p.air) {
			last_block = b;
			dist++;
			continue;
		}

		if (!put_block(b, dist, last_block == p.air))
			return false;
		dist = 0;
	}

	return full_flush();
}

bool ZsegWriter::put_palette(uint16_t id, const std::string& name) {
	uint8_t* wr = buf.write_ptr();
	int wcap = buf.write_cap();
	int n = name.length() + 1;

	if (wcap < n + 2) {
		cfn();
		err = true;
		return false;
	}

	memcpy(wr, name.c_str(), n);
	wr += n;

	uint16_t* tmp = (uint16_t*)wr;
	*tmp = id;

	buf.adv_write_ptr(n + 2);

	if (wcap - n - 2 < 256)
		partial_flush();

	return true;
}

bool ZsegWriter::end_palette() {
	uint8_t* wr = buf.write_ptr();
	int wcap = buf.write_cap();

	if (wcap < 3) {
		cfn();
		err = true;
		return false;
	}

	wr[0] = 0;
	wr[1] = 0;
	wr[2] = 0;

	buf.adv_write_ptr(3);

	return true;
}

bool ZsegWriter::put_block(uint16_t block, long int dist, bool diff_is_air) {
	uint8_t* wr = buf.write_ptr();
	int wcap = buf.write_cap();

	if (wcap < 12) {
		cfn();
		err = true;
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

bool ZsegWriter::full_flush() {
	const uint8_t* rp = buf.read_ptr();
	int rcap = buf.read_cap();

	ssize_t n = wfn(rp, rcap, true);

	buf.adv_read_ptr(n);

	return n == rcap;
}

bool ZsegWriter::partial_flush() {
	const uint8_t* rp = buf.read_ptr();
	int rcap = buf.read_cap();

	ssize_t n = wfn(rp, rcap, false);
	if (n < 0) {
		return false;
	}

	buf.adv_read_ptr(n);
	return true;
}
