#include <cstdio>
#include <cstdint>

#include "zseg-writer.hpp"
#include "region_reader.hpp"
#include "palette.hpp"

int main() {
	RegionReader rd("r.-1.0.mca");

	if (rd.err())
		return 1;

//	for (int i = 0; i < 16; i++) {
//		SegmentMinifier m(rd, i % 4, i / 4);
//		auto blocks = m.minify();
//	}

	SegmentMinifier m(rd, 2, 3);

	ZsegWriter wr(1, 2);
	if (wr.err())
		return 1;

	wr.put_minifier(m);
}
