#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "region_reader.hpp"
#include "zseg-writer.hpp"
#include "zstd-writer.hpp"

int open_file(int segx, int segz, bool compressed=false, int dirfd=AT_FDCWD) {
	char filename[64];
	sprintf(filename, "seg.%d.%d.%sseg", segx, segz, compressed? "z": "");

	return openat(dirfd, filename, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644);
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

	int fd = open_file(1, 2, true);
	ZstdWriter zwr(
		[=](const uint8_t* p, size_t n) {
			return write(fd, p, n);
		},
		[=]() {
			close(fd);
			return true;
		},
	15);

	ZsegWriter wr(
		[&] (const uint8_t* p, size_t n, bool end) {
			return zwr.write(p, n, end);
		},
		[&] () {
			zwr.full_flush();
			return true;
		}
	);

	if (wr.err)
		return 1;

	wr.put_minifier(m);
}
